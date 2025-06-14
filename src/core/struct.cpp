/**
 * Struct implementation adapted from the Mitsuba project
 */
#include <eldr/core/hash.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/core/struct.hpp>

#include <sstream>

NAMESPACE_BEGIN(eldr)
Struct::Struct(bool pack, ByteOrder byte_order)
  : pack_(pack), byte_order_(byte_order)
{
  if (byte_order_ == ByteOrder::HostByteOrder)
    byte_order_ = hostByteOrder();
}

Struct::Struct(const Struct& s)
  : fields_(s.fields_), pack_(s.pack_), byte_order_(s.byte_order_)
{
}

size_t Struct::size() const
{
  if (fields_.empty())
    return 0;
  auto const& last = fields_[fields_.size() - 1];
  size_t      size = last.offset + last.size;
  if (!pack_) {
    size_t align = alignment();
    size += (align - size) % align;
  }
  return size;
}

size_t Struct::alignment() const
{
  if (pack_)
    return 1;
  size_t size = 1;
  for (auto const& field : fields_)
    size = std::max(size, field.size);
  return size;
}

bool Struct::hasField(const std::string& name) const
{
  for (auto const& field : fields_)
    if (field.name == name)
      return true;
  return false;
}

Struct& Struct::append(const std::string&  name,
                       StructType          type,
                       StructPropertyFlags flags,
                       double              default_)
{
  Field f;
  f.name     = name;
  f.type     = type;
  f.flags    = flags;
  f.default_ = default_;
  if (fields_.empty()) {
    f.offset = 0;
  }
  else {
    auto const& last = fields_[fields_.size() - 1];
    f.offset         = last.offset + last.size;
  }
  switch (type) {
    case StructType::Int8:
    case StructType::UInt8:
      f.size = 1;
      break;
    case StructType::Int16:
    case StructType::UInt16:
    case StructType::Float16:
      f.size = 2;
      break;
    case StructType::Int32:
    case StructType::UInt32:
    case StructType::Float32:
      f.size = 4;
      break;
    case StructType::Int64:
    case StructType::UInt64:
    case StructType::Float64:
      f.size = 8;
      break;
    default:
      Throw("Struct::append(): invalid field type!");
  }
  if (!pack_)
    f.offset += (f.size - f.offset) % f.size;
  fields_.push_back(f);
  return *this;
}

std::ostream& operator<<(std::ostream& os, const StructType& value)
{
  switch (value) {
    case StructType::Int8:
      os << "int8";
      break;
    case StructType::UInt8:
      os << "uint8";
      break;
    case StructType::Int16:
      os << "int16";
      break;
    case StructType::UInt16:
      os << "uint16";
      break;
    case StructType::Int32:
      os << "int32";
      break;
    case StructType::UInt32:
      os << "uint32";
      break;
    case StructType::Int64:
      os << "int64";
      break;
    case StructType::UInt64:
      os << "uint64";
      break;
    case StructType::Float16:
      os << "float16";
      break;
    case StructType::Float32:
      os << "float32";
      break;
    case StructType::Float64:
      os << "float64";
      break;
    case StructType::Invalid:
      os << "invalid";
      break;
    default:
      Throw("Struct: operator<<: invalid field type!");
  }
  return os;
}

std::string Struct::toString() const
{
  std::ostringstream os;
  os << "Struct<" << size() << ">[" << std::endl;
  for (size_t i = 0; i < fields_.size(); ++i) {
    auto const& f = fields_[i];
    if (i > 0) {
      size_t padding = f.offset - (fields_[i - 1].offset + fields_[i - 1].size);
      if (padding > 0)
        os << "  // " << padding << " byte" << (padding > 1 ? "s" : "")
           << " of padding." << std::endl;
    }
    os << "  " << f.type;
    os << " " << f.name << "; // @" << f.offset;
    if (f.flags.hasFlag(StructProperty::Normalized))
      os << ", normalized";
    if (f.flags.hasFlag(StructProperty::Gamma))
      os << ", gamma";
    if (f.flags.hasFlag(StructProperty::Weight))
      os << ", weight";
    if (f.flags.hasFlag(StructProperty::Alpha))
      os << ", alpha";
    if (f.flags.hasFlag(StructProperty::PremultipliedAlpha))
      os << ", premultiplied alpha";
    if (f.flags.hasFlag(StructProperty::Default))
      os << ", default=" << f.default_;
    if (f.flags.hasFlag(StructProperty::Assert))
      os << ", assert=" << f.default_;
    if (!f.blend.empty()) {
      os << ", blend = <";
      for (size_t j = 0; j < f.blend.size(); ++j) {
        os << f.blend[j].second << " * " << f.blend[j].first;
        if (j + 1 < f.blend.size())
          os << " + ";
      }
      os << ">";
    }
    os << "\n";
  }
  if (!fields_.empty()) {
    size_t padding = size() - (fields_[fields_.size() - 1].offset +
                               fields_[fields_.size() - 1].size);
    if (padding > 0)
      os << "  // " << padding << " byte" << (padding > 1 ? "s" : "")
         << " of padding." << std::endl;
  }
  os << "]";
  return os.str();
}

const Struct::Field& Struct::field(const std::string& name) const
{
  for (auto const& field : fields_)
    if (field.name == name)
      return field;
  Throw("Unable to find field \"%s\"", name);
}

Struct::Field& Struct::field(const std::string& name)
{
  for (auto& field : fields_)
    if (field.name == name)
      return field;
  Throw("Unable to find field \"%s\"", name);
}

std::pair<double, double> Struct::range(StructType type)
{
  std::pair<double, double> result;

#define COMPUTE_RANGE(key, type)                                               \
  case key:                                                                    \
    result = std::make_pair((double) std::numeric_limits<type>::min(),         \
                            (double) std::numeric_limits<type>::max());        \
    break;

  switch (type) {
    COMPUTE_RANGE(StructType::UInt8, uint8_t);
    COMPUTE_RANGE(StructType::Int8, int8_t);
    COMPUTE_RANGE(StructType::UInt16, uint16_t);
    COMPUTE_RANGE(StructType::Int16, int16_t);
    COMPUTE_RANGE(StructType::UInt32, uint32_t);
    COMPUTE_RANGE(StructType::Int32, int32_t);
    COMPUTE_RANGE(StructType::UInt64, uint64_t);
    COMPUTE_RANGE(StructType::Int64, int64_t);
    COMPUTE_RANGE(StructType::Float32, float);
    COMPUTE_RANGE(StructType::Float64, double);

    case StructType::Float16:
      result = std::make_pair(-65504, 65504);
      break;

    default:
      Throw("Internal error: invalid field type");
  }

  if (isInteger(type)) {
    // Account for rounding errors in the conversions above.
    // (we want the bounds to be conservative)
    if (result.first != 0)
      result.first = std::nextafter(
        result.first, result.first + std::numeric_limits<double>::epsilon());
    result.second = std::nextafter(
      result.second, result.second - std::numeric_limits<double>::epsilon());
  }

  return result;
}

size_t hash(const Struct::Field& f)
{
  size_t value{ hash(f.name) };
  value = hashCombine(value, hash(f.type));
  value = hashCombine(value, hash(f.size));
  value = hashCombine(value, hash(f.offset));
  value = hashCombine(value, hash(f.flags));
  value = hashCombine(value, hash(f.default_));
  return value;
}

size_t hash(const Struct& s)
{
  return hashCombine(hashCombine(hash(s.fields_), hash(s.pack_)),
                     hash(s.byte_order_));
}
NAMESPACE_END(eldr)
