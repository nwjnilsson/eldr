/**
 * MemoryStream class adapted from Mitsuba3
 */
#pragma once

#include <eldr/core/stream.hpp>

NAMESPACE_BEGIN(eldr)

/** \brief Simple memory buffer-based stream with automatic memory management.
 * It always has read & write capabilities.
 *
 * The underlying memory storage of this implementation dynamically expands
 * as data is written to the stream, à la <tt>std::vector</tt>.
 */
class MemoryStream : public Stream {
public:
  using Stream::read;
  using Stream::write;

  /** \brief Creates a new memory stream, initializing the memory buffer
   * with a capacity of <tt>capacity</tt> bytes. For best performance,
   * set this argument to the estimated size of the content that
   * will be written to the stream.
   */
  MemoryStream(size_t capacity = 512);

  /**
   * \brief Creates a memory stream, which operates on a pre-allocated buffer.
   *
   * A memory stream created in this way will never resize the
   * underlying buffer. An exception is thrown e.g. when attempting
   * to extend its size.
   *
   */
  MemoryStream(void* ptr, size_t size);

  /// Virtual destructor
  virtual ~MemoryStream();

  /// Returns a string representation
  std::string toString() const override;

  /** \brief Closes the stream.
   * No further read or write operations are permitted.
   *
   * This function is idempotent.
   * It may be called automatically by the destructor.
   */
  virtual void close() override { is_closed_ = true; };

  /// Whether the stream is closed (no read or write are then permitted).
  virtual bool isClosed() const override { return is_closed_; };

  /**
   * \brief Reads a specified amount of data from the stream.
   * Throws an exception if trying to read further than the current size
   * of the contents.
   */
  virtual void read(void* p, size_t size) override;

  /**
   * \brief Writes a specified amount of data into the memory buffer.
   * The capacity of the memory buffer is extended if necessary.
   */
  virtual void write(const void* p, size_t size) override;

  /** Seeks to a position inside the stream.
   * You may seek beyond the size of the stream's contents, or even beyond the
   * buffer's capacity. The size and capacity are <b>not</b> affected.
   * A subsequent write would then expand the size and capacity
   * accordingly. The contents of the memory that was skipped is undefined.
   */
  virtual void seek(size_t pos) override { pos_ = pos; }

  /** \brief Truncates the contents <b>and</b> the memory buffer's capacity
   * to a given size.
   * The position is updated to <tt>min(old_position, size)</tt>.
   *
   * \note This will throw is the MemoryStream was initialized with a
   * pre-allocated buffer.
   */
  virtual void truncate(size_t size) override;

  /** \brief Gets the current position inside the memory buffer. Note that
   * this might be further than the stream's size or even capacity.
   */

  virtual size_t tell() const override { return pos_; };

  /** \brief Returns the size of the contents written to the memory buffer.
   * \note This is not equal to the size of the memory buffer in general,
   * since we allocate more capacity at once.
   */
  virtual size_t size() const override { return size_; };

  /// No-op since all writes are made directly to memory
  virtual void flush() override {};

  /// Always returns true, except if the stream is closed.
  virtual bool canWrite() const override { return !isClosed(); }

  /// Always returns true, except if the stream is closed.
  virtual bool canRead() const override { return !isClosed(); }

  /// Return the current capacity of the underlying memory buffer
  size_t capacity() const { return capacity_; }

  /// Return whether or not the memory stream owns the underlying buffer
  bool ownsBuffer() const { return owns_buffer_; }

  /// Return the underlying raw byte array
  const byte_t* rawBuffer() const { return data_; }

  //! @}
  // =========================================================================

protected:
  void resize(size_t newSize);

private:
  /// Current size of the allocated memory buffer
  size_t capacity_;
  /// Current size of the contents written to the memory buffer
  size_t size_;
  /// Current position inside of the memory buffer
  size_t pos_;
  /// False if the MemoryStream was created from a pre-allocated buffer
  bool owns_buffer_;
  /// Pointer to the memory buffer (might not be owned)
  byte_t* data_;
  /// Whether the stream has been closed.
  bool is_closed_;
};
NAMESPACE_END(eldr)
