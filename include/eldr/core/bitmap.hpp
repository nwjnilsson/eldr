/**
 * Bitmap class adapted from Mitsuba3
 */
#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/core/struct.hpp>

#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace eldr {

class Bitmap {
  ELDR_IMPORT_CORE_TYPES();

public:
  enum class PixelFormat {
    Y,
    YA, // Gray alpha
    RGB,
    RGBA,
    RGBW,
    RGBAW,
    XYZ,
    XYZA,
    MultiChannel
  };

  // friend struct fmt::formatter<PixelFormat>;

  enum class FileFormat {
    PNG,
    OpenEXR,
    JPEG,
    BMP,
    PFM,
    PPM,
    RGBE,
    Auto,
    Unknown
  };

  Bitmap(std::string_view                name,
         PixelFormat                     px_format,
         StructType                      component_format,
         Vec2u                           size,
         size_t                          channel_count,
         const std::vector<std::string>& channel_names = {},
         byte_t*                         data          = nullptr);

  Bitmap(const std::filesystem::path& path, FileFormat = FileFormat::Auto);

  Bitmap(const Bitmap& bitmap);

  Bitmap(Bitmap&& bitmap) noexcept;

  Bitmap(Stream* stream, FileFormat format);

  virtual ~Bitmap();

  /// Return the name of this bitmap
  const std::string& name() const { return name_; }

  /// Return the pixel format of this bitmap
  PixelFormat pixelFormat() const { return pixel_format_; }

  /// Return the component format of this bitmap
  StructType componentFormat() const { return component_format_; }

  /// Return a pointer to the underlying data
  byte_t* data() { return data_.get(); }

  /// Return a pointer to the underlying data (const)
  const byte_t* data() const { return data_.get(); }

  /// Return a span of the underlying data
  std::span<byte_t> bytes() { return std::span{ data_.get(), bufferSize() }; }

  /// Return a span of the underlying data (const)
  std::span<const byte_t> bytes() const
  {
    return std::span{ data_.get(), bufferSize() };
  }

  /// Return the bitmap dimensions in pixels
  const Vec2u& size() const { return size_; }

  /// Return the bitmap's width in pixels
  uint32_t width() const { return size_.x; }

  /// Return the bitmap's height in pixels
  uint32_t height() const { return size_.y; }

  size_t channelCount() const { return struct_->fieldCount(); }
  size_t pixelCount() const { return static_cast<size_t>(size_.x * size_.y); }
  size_t bytesPerPixel() const;
  size_t bufferSize() const;
  bool   srgbGamma() const { return srgb_gamma_; }
  // Convert RGB to RGBA, adding an opaque alpha channel.
  // This is useful for creating Vulkan images, which require RGBA.
  void              rgbToRgba();
  static FileFormat detectFileFormat(Stream* stream);

  // Creates default checker tile texture
  static Bitmap createCheckerboard();
  // Creates default white texture
  static Bitmap createDefaultWhite();

protected:
  void rebuildStruct(size_t                          channel_count = 0,
                     const std::vector<std::string>& channel_names = {});
  /// Read a file from a stream
  void read(Stream* stream, FileFormat format);

  /// Read a file encoded using the OpenEXR file format
  // void read_exr(Stream* stream);

  ///// Write a file using the OpenEXR file format
  // void write_exr(Stream* stream, int compression = -1) const;

  ///// Read a file encoded using the JPEG file format
  void readJpeg(Stream* stream);

  ///// Save a file using the JPEG file format
  // void write_jpeg(Stream* stream, int quality) const;

  // Read a file encoded using the PNG file format
  void readPng(Stream* stream);

  ///// Save a file using the PNG file format
  // void write_png(Stream* stream, int quality) const;

  ///// Read a file encoded using the PPM file format
  // void read_ppm(Stream* stream);

  ///// Save a file using the PPM file format
  // void write_ppm(Stream* stream) const;

  ///// Read a file encoded using the BMP file format
  // void read_bmp(Stream* stream);

  ///// Read a file encoded using the TGA file format
  // void read_tga(Stream* stream);

  ///// Read a file encoded using the RGBE file format
  // void read_rgbe(Stream* stream);

  ///// Save a file using the RGBE file format
  // void write_rgbe(Stream* stream) const;

  ///// Read a file encoded using the PFM file format
  // void read_pfm(Stream* stream);

  ///// Save a file using the PFM file format
  // void write_pfm(Stream* stream) const;

private:
  std::string               name_{ "undefined" };
  PixelFormat               pixel_format_;
  StructType                component_format_;
  Vec2u                     size_{};
  std::unique_ptr<Struct>   struct_{};
  bool                      srgb_gamma_{ false };
  bool                      premultiplied_alpha_{ false };
  std::unique_ptr<byte_t[]> data_{};
  bool                      owns_data_{ false };
};

extern std::ostream& operator<<(std::ostream&              os,
                                const Bitmap::PixelFormat& value);
extern std::ostream& operator<<(std::ostream&             os,
                                const Bitmap::FileFormat& value);
} // namespace eldr
template <>
struct fmt::formatter<eldr::Bitmap::PixelFormat> : fmt::ostream_formatter {};
template <>
struct fmt::formatter<eldr::Bitmap::FileFormat> : fmt::ostream_formatter {};
