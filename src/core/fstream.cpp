/**
 * FileStream implementation adapted from Mitsuba3
 */
#include <eldr/core/fstream.hpp>
#include <eldr/core/platform.hpp>

#include <fstream>

namespace eldr {

namespace detail {
inline std::ios::openmode ios_flag(FileStream::EMode mode)
{
  switch (mode) {
    case FileStream::ERead:
      return std::ios::binary | std::ios::in;
    case FileStream::EReadWrite:
      return std::ios::binary | std::ios::in | std::ios::out;
    case FileStream::ETruncReadWrite:
      return std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc;
    default:
      Throw("Internal error");
  }
}
} // namespace detail

FileStream::FileStream(const fs::path& p, EMode mode)
  : Stream(), mode_(mode), path_(p), file_(std::make_unique<std::fstream>())
{

  file_->open(p.string(), detail::ios_flag(mode));

  if (!file_->good())
    Throw("\"%s\": I/O error while attempting to open file: %s", path_.string(),
          strerror(errno));
}

FileStream::~FileStream() { close(); }

void FileStream::close() { file_->close(); };

bool FileStream::isClosed() const { return !file_->is_open(); };

void FileStream::read(void* p, size_t size)
{
  file_->read((char*) p, size);

  if (unlikely(!file_->good())) {
    bool   eof    = file_->eof();
    size_t gcount = file_->gcount();
    file_->clear();
    if (eof)
      throw EOFException(fmt::format("\"%s\": read %zu out of %zu bytes",
                                     path_.string(), gcount, size),
                         gcount);
    else
      Throw("\"%s\": I/O error while attempting to read %zu bytes: %s",
            path_.string(), size, strerror(errno));
  }
}

void FileStream::write(const void* p, size_t size)
{
  file_->write((char*) p, size);

  if (unlikely(!file_->good())) {
    file_->clear();
    Throw("\"%s\": I/O error while attempting to write %zu bytes: %s",
          path_.string(), size, strerror(errno));
  }
}

void FileStream::truncate(size_t size)
{
  if (mode_ == ERead) {
    Throw("\"%s\": attempting to truncate a read-only FileStream",
          path_.string());
  }

  flush();
  const size_t old_pos = tell();
#if defined(_WIN32)
  // Windows won't allow a resize if the file is open
  close();
#else
  seek(0);
#endif

  fs::resize_file(path_, size);

#if defined(_WIN32)
  file_->open(path_, detail::ios_flag(EReadWrite));
  if (!file_->good())
    Throw("\"%s\": I/O error while attempting to open file: %s", path_.string(),
          strerror(errno));
#endif

  seek(std::min(old_pos, size));
}

void FileStream::seek(size_t pos)
{
  file_->seekg(static_cast<std::ios::pos_type>(pos));

  if (unlikely(!file_->good()))
    Throw("\"%s\": I/O error while attempting to seek to offset %zu: %s",
          path_.string(), pos, strerror(errno));
}

size_t FileStream::tell() const
{
  std::ios::pos_type pos = file_->tellg();
  if (unlikely(pos == std::ios::pos_type(-1)))
    Throw("\"%s\": I/O error while attempting to determine "
          "position in file",
          path_.string());
  return static_cast<size_t>(pos);
}

void FileStream::flush()
{
  file_->flush();
  if (unlikely(!file_->good())) {
    file_->clear();
    Throw("\"%s\": I/O error while attempting flush "
          "file stream: %s",
          path_.string(), strerror(errno));
  }
}

size_t FileStream::size() const { return fs::file_size(path_); }

std::string FileStream::readLine()
{
  std::string result;
  if (!std::getline(*file_, result))
    spdlog::error("\"%s\": I/O error while attempting to read a line of text: %s",
        path_.string(), strerror(errno));
  return result;
}

std::string FileStream::toString() const
{
  std::ostringstream oss;

  oss << "FileStream" << "[" << std::endl;
  if (isClosed()) {
    oss << "  closed" << std::endl;
  }
  else {
    size_t pos = (size_t) -1;
    try {
      pos = tell();
    }
    catch (...) {
    }
    oss << "  path_ = \"" << path_.string() << "\""
        << "," << std::endl
        << "  host_byte_order = " << host_byte_order() << "," << std::endl
        << "  byte_order = " << byteOrder() << "," << std::endl
        << "  can_read = " << canRead() << "," << std::endl
        << "  can_write = " << canWrite() << "," << std::endl
        << "  pos = " << pos << "," << std::endl
        << "  size = " << size() << std::endl;
  }

  oss << "]";

  return oss.str();
}
} // namespace eldr
