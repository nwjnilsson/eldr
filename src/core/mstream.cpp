/**
 * MemoryStream class adapted from Mitsuba3
 */
#include <eldr/core/common.hpp>
#include <eldr/core/mstream.hpp>

#include <sstream>

namespace eldr::core {

MemoryStream::MemoryStream(size_t capacity)
  : Stream(), capacity_(0), size_(0), pos_(0), owns_buffer_(true),
    data_(nullptr), is_closed_(false)
{
  resize(capacity);
}

MemoryStream::MemoryStream(void* ptr, size_t size)
  : Stream(), capacity_(size), size_(size), pos_(0), owns_buffer_(false),
    data_(reinterpret_cast<uint8_t*>(ptr)), is_closed_(false)
{
}

MemoryStream::~MemoryStream()
{
  if (data_ != nullptr && owns_buffer_)
    std::free(data_);
}

void MemoryStream::read(void* p, size_t size)
{
  if (isClosed())
    Throw("Attempted to read from a closed stream: %s", toString());

  if (pos_ + size > size_) {
    // Use signed difference since `pos_` might be beyond `size_`
    int64_t size_read = size_ - static_cast<int64_t>(pos_);
    if (size_read > 0) {
      memcpy(p, data_ + pos_, static_cast<size_t>(size_read));
      pos_ += static_cast<size_t>(size_read);
    }
  }
  else {
    memcpy(p, data_ + pos_, size);
    pos_ += size;
  }
}

void MemoryStream::write(const void* p, size_t size)
{
  if (isClosed())
    Throw("Attempted to write to a closed stream: %s", toString());

  size_t endPos = pos_ + size;
  if (endPos > size_) {
    if (endPos > capacity_) {
      // Double capacity until it will fit `endPos`, à la `std::vector`
      auto newSize = capacity_;
      do {
        newSize *= 2;
      } while (endPos > newSize);
      resize(newSize);
    }
    size_ = endPos;
  }
  memcpy(data_ + pos_, p, size);
  pos_ = endPos;
}

void MemoryStream::resize(size_t size)
{
  if (!owns_buffer_)
    Throw("Tried to resize a buffer, which doesn't "
          "belong to this MemoryStream instance!");

  if (data_ == nullptr)
    data_ = reinterpret_cast<uint8_t*>(std::malloc(size));
  else
    data_ = reinterpret_cast<uint8_t*>(std::realloc(data_, size));

  if (size > capacity_)
    memset(data_ + capacity_, 0, size - capacity_);

  capacity_ = size;
}

void MemoryStream::truncate(size_t size)
{
  size_ = size;
  resize(size);
  if (pos_ > size_)
    pos_ = size_;
}

std::string MemoryStream::toString() const
{
  std::ostringstream oss;

  oss << "MemoryStream" << "[" << std::endl;
  if (isClosed()) {
    oss << "  closed" << std::endl;
  }
  else {
    oss << "  host_byte_order = " << host_byte_order() << "," << std::endl
        << "  byte_order = " << byteOrder() << "," << std::endl
        << "  can_read = " << canRead() << "," << std::endl
        << "  can_write = " << canWrite() << "," << std::endl
        << "  owns_buffer = " << owns_buffer() << "," << std::endl
        << "  capacity = " << capacity() << "," << std::endl
        << "  pos = " << tell() << "," << std::endl
        << "  size = " << size() << std::endl;
  }

  oss << "]";

  return oss.str();
}

} // namespace eldr::core
