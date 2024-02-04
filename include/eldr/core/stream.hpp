#pragma once

#include <eldr/core/logger.hpp>

/**
 * Abstract seekable stream class. Adapted from the Mitsuba project.
 *
 * Specifies all functions to be implemented by stream
 * subclasses and provides various convenience functions
 * layered on top of on them.
 *
 * All ``read*()`` and ``write*()`` methods support transparent
 * conversion based on the endianness of the underlying system and the
 * value passed to \ref setByteOrder(). Whenever \ref hostByteOrder()
 * and \ref byteOrder() disagree, the endianness is swapped.
 *
 * \sa FileStream, MemoryStream, DummyStream
 */

namespace eldr {
namespace detail {
template <typename T, typename SFINAE = void> struct serialization_helper;
} // namespace detail
class Stream {

public:
  enum EByteOrder {
    EBigEndian        = 0,
    ELittleEndian     = 1,
    ENetworkByteOrder = EBigEndian
  };

  Stream();

  virtual std::string toString() const = 0;
  virtual void        close()          = 0;
  virtual bool        isClosed() const = 0;

  /**
   * \brief Reads a specified amount of data from the stream.
   * \note This does <b>not</b> handle endianness swapping.
   *
   * Throws an exception when the stream ended prematurely.
   * Implementations need to handle endianness swap when appropriate.
   */
  virtual void read(void* p, size_t size) = 0;

  /**
   * \brief Writes a specified amount of data into the stream.
   * \note This does <b>not</b> handle endianness swapping.
   *
   * Throws an exception when not all data could be written.
   * Implementations need to handle endianness swap when appropriate.
   */
  virtual void write(const void* p, size_t size) = 0;

  /** \brief Seeks to a position inside the stream.
   *
   * Seeking beyond the size of the buffer will not modify the length of
   * its contents. However, a subsequent write should start at the sought
   * position and update the size appropriately.
   */
  virtual void seek(size_t pos) = 0;

  /** \brief Truncates the stream to a given size.
   *
   * The position is updated to <tt>min(old_position, size)</tt>.
   * Throws an exception if in read-only mode.
   */
  virtual void truncate(size_t size) = 0;

  /// Gets the current position inside the stream
  virtual size_t tell() const = 0;

  /// Returns the size of the stream
  virtual size_t size() const = 0;

  /// Flushes the stream's buffers, if any
  virtual void flush() = 0;

  /// Can we write to the stream?
  virtual bool canWrite() const = 0;

  /// Can we read from the stream?
  virtual bool canRead() const = 0;

  /// @}
  // =========================================================================

  // =========================================================================
  //! @{ \name Read and write values
  // =========================================================================

  /**
   * \brief Reads one object of type T from the stream at the current position
   * by delegating to the appropriate <tt>serialization_helper</tt>.
   *
   * Endianness swapping is handled automatically if needed.
   */
  template <typename T> void read(T& value)
  {
    using helper = detail::serialization_helper<T>;
    helper::read(*this, &value, 1, needsEndianessSwap());
  }

  /**
   * \brief Reads multiple objects of type T from the stream at the current
   * position by delegating to the appropriate <tt>serialization_helper</tt>.
   *
   * Endianness swapping is handled automatically if needed.
   */
  template <typename T> void readArray(T* value, size_t count)
  {
    using helper = detail::serialization_helper<T>;
    helper::read(*this, value, count, needsEndianessSwap());
  }

  /**
   * \brief Reads one object of type T from the stream at the current position
   * by delegating to the appropriate <tt>serialization_helper</tt>.
   *
   * Endianness swapping is handled automatically if needed.
   */
  template <typename T> void write(const T& value)
  {
    using helper = detail::serialization_helper<T>;
    helper::write(*this, &value, 1, needsEndianessSwap());
  }

  /**
   * \brief Reads multiple objects of type T from the stream at the current
   * position by delegating to the appropriate <tt>serialization_helper</tt>.
   *
   * Endianness swapping is handled automatically if needed.
   */
  template <typename T> void write_array(const T* value, size_t count)
  {
    using helper = detail::serialization_helper<T>;
    helper::write(*this, value, count, needsEndianessSwap());
  }

  /// Convenience function for reading a line of text from an ASCII file
  virtual std::string readLine();

  /// Convenience function for reading a contiguous token from an ASCII file
  virtual std::string readToken();

  /// Convenience function for writing a line of text to an ASCII file
  void writeLine(const std::string& text);

  /// Skip ahead by a given number of bytes
  void skip(size_t amount);

  /// @}
  // =========================================================================

  // =========================================================================
  //! @{ \name Endianness handling
  // =========================================================================

  /** \brief Sets the byte order to use in this stream.
   *
   * Automatic conversion will be performed on read and write operations
   * to match the system's native endianness.
   *
   * No consistency is guaranteed if this method is called after
   * performing some read and write operations on the system using a
   * different endianness.
   */
  void set_byte_order(EByteOrder byte_order);

  /// Returns the byte order of this stream.
  EByteOrder byteOrder() const { return byte_order_; }

  /// Returns true if we need to perform endianness swapping before writing or
  /// reading.
  bool needsEndianessSwap() const
  {
    return byte_order_ != host_byte_order_;
  }

  /// Returns the byte order of the underlying machine.
  static EByteOrder host_byte_order() { return host_byte_order_; }

  /// @}
  // =========================================================================

protected:
  /// Destructor
  virtual ~Stream();

  /// Copying is disallowed.
  Stream(const Stream&)         = delete;
  void operator=(const Stream&) = delete;

private:
  static const EByteOrder host_byte_order_;
  EByteOrder              byte_order_;
};

namespace detail {
} // namespace detail
} // namespace eldr
