/**
 * Stream implementation adapted from the Mitsuba project
 */
#include <eldr/core/stream.hpp>

#include <sstream>

namespace eldr::core {
namespace detail {
static Stream::EByteOrder byteOrder()
{
  union {
    uint8_t  char_value[2];
    uint16_t short_value;
  };
  char_value[0] = 1;
  char_value[1] = 0;

  if (short_value == 1)
    return Stream::ELittleEndian;
  else
    return Stream::EBigEndian;
}
} // namespace detail

const Stream::EByteOrder Stream::host_byte_order_ = detail::byteOrder();

// -----------------------------------------------------------------------------

Stream::Stream() : byte_order_(host_byte_order_) {}

Stream::~Stream() {}

void Stream::setByteOrder(EByteOrder value) { byte_order_ = value; }

// -----------------------------------------------------------------------------

std::string Stream::toString() const
{
  std::ostringstream oss;

  oss << "Stream" << "[" << std::endl;
  if (isClosed()) {
    oss << "  closed" << std::endl;
  }
  else {
    oss << "  host_byte_order = " << host_byte_order_ << "," << std::endl
        << "  byte_order = " << byte_order_ << "," << std::endl
        << "  can_read = " << canRead() << "," << std::endl
        << "  can_write = " << canWrite() << "," << std::endl
        << "  pos = " << tell() << "," << std::endl
        << "  size = " << size() << std::endl;
  }

  oss << "]";

  return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Stream::EByteOrder& value)
{
  switch (value) {
    case Stream::ELittleEndian:
      os << "little-endian";
      break;
    case Stream::EBigEndian:
      os << "big-endian";
      break;
    default:
      os << "invalid";
      break;
  }
  return os;
}

void Stream::writeLine(const std::string& text)
{
  write(text.data(), text.length() * sizeof(char));
  write('\n');
}

std::string Stream::readLine()
{
  std::string result;
  result.reserve(80);

  try {
    do {
      char data;
      read(&data, sizeof(char));
      if (data == '\n')
        break;
      else if (data != '\r')
        result += data;
    } while (true);
  }
  catch (...) {
    if (tell() == size() && !result.empty())
      return result;
    throw;
  }

  return result;
}

std::string Stream::readToken()
{
  std::string result;

  try {
    do {
      char data;
      read(&data, sizeof(char));
      if (std::isspace(data)) {
        if (result.empty())
          continue;
        else
          break;
      }
      result += data;
    } while (true);
  }
  catch (...) {
    if (tell() == size() && !result.empty())
      return result;
    throw;
  }

  return result;
}

void Stream::skip(size_t amount) { seek(tell() + amount); }
} // namespace eldr::core
