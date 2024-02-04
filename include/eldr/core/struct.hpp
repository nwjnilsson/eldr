#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace eldr {
class Struct {
public:
  enum class Type : uint32_t {
    // Invalid/unspecified
    Invalid = 0,

    // Signed and unsigned integer values
    UInt8,
    Int8,
    UInt16,
    Int16,
    UInt32,
    Int32,
    UInt64,
    Int64,

    // Floating point values
    Float16,
    Float32,
    Float64,
  };

  enum class ByteOrder { LittleEndian, BigEndian, HostByteOrder };
  /// Field-specific flags
  enum class Flags : uint32_t {
    /// No flags set (default value)
    Empty = 0x00,

    /**
     * Specifies whether an integer field encodes a normalized value in the
     * range [0, 1]. The flag is ignored if specified for floating point
     * valued fields.
     */
    Normalized = 0x01,

    /**
     * Specifies whether the field encodes a sRGB gamma-corrected value.
     * Assumes \c Normalized is also specified.
     */
    Gamma = 0x02,

    /**
     * In \ref FieldConverter::convert, check that the field value matches
     * the specified default value. Otherwise, return a failure
     */
    Assert = 0x04,

    /**
     * In \ref FieldConverter::convert, when the field is missing in the
     * source record, replace it by the specified default value
     */
    Default = 0x08,

    /**
     * In \ref FieldConverter::convert, when an input structure contains a
     * weight field, the value of all entries are considered to be
     * expressed relative to its value. Converting to an un-weighted
     * structure entails a division by the weight.
     */
    Weight = 0x10,

    /**
     * Specifies whether the field encodes an alpha premultiplied value
     */
    PremultipliedAlpha = 0x20,

    /**
     * Specifies whether the field encodes an alpha value
     */
    Alpha = 0x40
  };

  /// Field specifier with size and offset
  struct Field {
    /// Name of the field
    std::string name;

    /// Type identifier
    Type type;

    /// Size in bytes
    size_t size;

    /// Offset within the \c Struct (in bytes)
    size_t offset;

    /// Additional flags
    uint32_t flags;

    /// Default value
    double default_;

    /**
     * \brief For use with \ref StructConverter::convert()
     *
     * Specifies a pair of weights and source field names that will be
     * linearly blended to obtain the output field value. Note that this
     * only works for floating point fields or integer fields with the \ref
     * Flags::Normalized flag. Gamma-corrected fields will be blended in linear
     * space.
     */
    std::vector<std::pair<double, std::string>> blend;

    /// Return a hash code associated with this \c Field
    friend size_t hash(const Field& f);

    /// Equality operator
    bool operator==(const Field& f) const
    {
      return name == f.name && type == f.type && size == f.size &&
             offset == f.offset && flags == f.flags && default_ == f.default_;
    }

    /// Equality operator
    bool operator!=(const Field& f) const { return !operator==(f); }

    bool isUnsigned() const { return Struct::isUnsigned(type); }

    bool isSigned() const { return Struct::isSigned(type); }

    bool isFloat() const { return Struct::isFloat(type); }

    bool isInteger() const { return Struct::isInteger(type); }

    std::pair<double, double> range() const { return Struct::range(type); }
  };

  using FieldIterator      = std::vector<Field>::iterator;
  using FieldConstIterator = std::vector<Field>::const_iterator;

  /// Create a new \c Struct and indicate whether the contents are packed or
  /// aligned
  Struct(bool pack = false, ByteOrder byte_order = ByteOrder::HostByteOrder);

  /// Copy constructor
  Struct(const Struct& s);

  /// Append a new field to the \c Struct; determines size and offset
  /// automatically
  Struct& append(const std::string& name, Type type,
                 uint32_t flags    = (uint32_t) Flags::Empty,
                 double   default_ = 0.0);

  /// Append a new field to the \c Struct (manual version)
  Struct& append(Field field)
  {
    fields_.push_back(field);
    return *this;
  }

  /// Access an individual field entry
  const Field& operator[](size_t i) const { return fields_[i]; }

  /// Access an individual field entry
  Field& operator[](size_t i) { return fields_[i]; }

  /// Check if the \c Struct has a field of the specified name
  bool hasField(const std::string& name) const;

  /// Return the size (in bytes) of the data structure, including padding
  size_t size() const;

  /// Return the alignment (in bytes) of the data structure
  size_t alignment() const;

  /// Return the number of fields
  size_t fieldCount() const { return fields_.size(); }

  /// Return the byte order of the \c Struct
  ByteOrder byteOrder() const { return byte_order_; }

  /// Return the byte order of the host machine
  static ByteOrder host_byte_order()
  {
#if defined(LITTLE_ENDIAN)
    return ByteOrder::LittleEndian;
#elif defined(BIG_ENDIAN)
    return ByteOrder::LittleEndian;
#else
#error Either LITTLE_ENDIAN or BIG_ENDIAN must be defined!
#endif
  };

  /// Look up a field by name (throws an exception if not found)
  const Field& field(const std::string& name) const;

  /// Look up a field by name. Throws an exception if not found
  Field& field(const std::string& name);

  /// Return the offset of the i-th field
  size_t offset(size_t i) const { return operator[](i).offset; }

  /// Return the offset of field with the given name
  size_t offset(const std::string& name) const { return field(name).offset; }

  /// Return an iterator associated with the first field
  FieldConstIterator begin() const { return fields_.cbegin(); }

  /// Return an iterator associated with the first field
  FieldIterator begin() { return fields_.begin(); }

  /// Return an iterator associated with the end of the data structure
  FieldConstIterator end() const { return fields_.cend(); }

  /// Return an iterator associated with the end of the data structure
  FieldIterator end() { return fields_.end(); }

  /// Return a hash code associated with this \c Struct
  friend size_t hash(const Struct& s);

  /// Equality operator
  bool operator==(const Struct& s) const
  {
    return fields_ == s.fields_ && pack_ == s.pack_ &&
           byte_order_ == s.byte_order_;
  }

  /// Inequality operator
  bool operator!=(const Struct& s) const { return !operator==(s); }

  /// Return a string representation
  std::string to_string() const; // override;

  /// Check whether the given type is an unsigned type
  static bool isUnsigned(Type type)
  {
    return type == Type::UInt8 || type == Type::UInt16 ||
           type == Type::UInt32 || type == Type::UInt64;
  }

  /// Check whether the given type is a signed type
  static bool isSigned(Type type) { return !isUnsigned(type); }

  /// Check whether the given type is an integer type
  static bool isInteger(Type type) { return !isFloat(type); }

  /// Check whether the given type is a floating point type
  static bool isFloat(Type type)
  {
    return type == Type::Float16 || type == Type::Float32 ||
           type == Type::Float64;
  }

  /// Return the representable range of the given type
  static std::pair<double, double> range(Type type);

protected:
  std::vector<Field> fields_;
  bool               pack_;
  ByteOrder          byte_order_;
};

constexpr const char* toString(Struct::Type type)
{
  switch (type) {
    case Struct::Type::Invalid:
      return "Invalid";
    case Struct::Type::UInt8:
      return "UInt8";
    case Struct::Type::Int8:
      return "Int8";
    case Struct::Type::UInt16:
      return "UInt16";
    case Struct::Type::Int16:
      return "Int16";
    case Struct::Type::UInt32:
      return "UInt32";
    case Struct::Type::Int32:
      return "Int32";
    case Struct::Type::UInt64:
      return "UInt64";
    case Struct::Type::Int64:
      return "Int64";
    case Struct::Type::Float16:
      return "Float16";
    case Struct::Type::Float32:
      return "Float32";
    case Struct::Type::Float64:
      return "Float64";
    default:
      throw std::invalid_argument("Struct type not implemented!");
  }
}

} // namespace eldr
