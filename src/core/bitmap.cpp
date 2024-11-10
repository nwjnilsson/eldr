/**
 * Bitmap implementation adapted from the Mitsuba3
 */
#include <eldr/core/logger.hpp>

#include <eldr/core/bitmap.hpp>
#include <eldr/core/fstream.hpp>

extern "C" {
#include <jerror.h>
#include <jpeglib.h>
}

#include <png.h>

#include <array>
#include <memory>
#include <stdexcept>
#include <string>

namespace eldr {

Bitmap::Bitmap()
  : pixel_format_(PixelFormat::RGBA), component_format_(Struct::Type::UInt8),
    size_({ 512, 512 }), srgb_gamma_(true)
{
  rebuildStruct();

  assert(!data_);
  data_ = std::make_unique<uint8_t[]>(bufferSize());
  // data_      = std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize()]);
  owns_data_ = true;

  constexpr uint32_t n_channels = 4;
  // Create an 8x8 checkerboard pattern of squares.
  constexpr uint32_t square_dimension{ 64 };
  // pink, purple
  constexpr std::array<std::array<unsigned char, 4>, 2> colors{
    { { 0xFF, 0x69, 0xB4, 0xFF }, { 0x94, 0x00, 0xD3, 0xFF } }
  };

  const auto getColor = [](uint32_t x, uint32_t y, uint32_t square_dimension,
                           size_t colors) -> int {
    return static_cast<int>((static_cast<std::size_t>(x / square_dimension) +
                             static_cast<std::size_t>(y / square_dimension)) %
                            colors);
  };

  // Performance could be improved by copying complete rows after one or two
  // rows are created with the loops.
  for (uint32_t y = 0; y < height(); y++) {
    for (uint32_t x = 0; x < width(); x++) {
      const int color_id = getColor(x, y, square_dimension, colors.size());
      std::memcpy(data_.get(), &colors[color_id][0],
                  n_channels * sizeof(colors[color_id][0]));
    }
  }
}

Bitmap::Bitmap(PixelFormat px_format, Struct::Type component_format,
               const Vec2u& size, size_t channel_count, const std::string& name,
               const std::vector<std::string>& channel_names, uint8_t* data)
  : name_(name), pixel_format_(px_format), component_format_(component_format),
    size_(size), data_(data)

{
  if (component_format_ == Struct::Type::UInt8)
    srgb_gamma_ = true;
  else
    srgb_gamma_ = false;

  if (!data_) {
    data_ = std::make_unique<uint8_t[]>(bufferSize());
    // data_      = std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize()]);
    owns_data_ = true;
  }
}

Bitmap::Bitmap(const Bitmap& bitmap)
  : name_(bitmap.name_), pixel_format_(bitmap.pixel_format_),
    component_format_(bitmap.component_format_), size_(bitmap.size_),
    struct_(std::make_unique<Struct>(*bitmap.struct_.get())),
    srgb_gamma_(bitmap.srgb_gamma_),
    premultiplied_alpha_(bitmap.premultiplied_alpha_), owns_data_(true)
{
  size_t size = bufferSize();
  data_       = std::make_unique<uint8_t[]>(size);
  memcpy(data_.get(), bitmap.data_.get(), size);
}

// TODO: fix
Bitmap::Bitmap(Bitmap&& bitmap)
  : name_(bitmap.name_), pixel_format_(bitmap.pixel_format_),
    component_format_(bitmap.component_format_), size_(bitmap.size_),
    struct_(std::move(bitmap.struct_)), srgb_gamma_(bitmap.srgb_gamma_),
    premultiplied_alpha_(bitmap.premultiplied_alpha_),
    data_(std::move(bitmap.data_)), owns_data_(bitmap.owns_data_)
{
  // use std::exchange....
}

Bitmap::Bitmap(Stream* stream, FileFormat format) { read(stream, format); }

Bitmap::Bitmap(const std::filesystem::path& path, FileFormat format)
{
  auto fs = std::make_unique<FileStream>(path);
  read(fs.get(), format);
}

Bitmap::~Bitmap()
{
  if (!owns_data_)
    data_.release();
}

void Bitmap::rebuildStruct(size_t                          channel_count,
                           const std::vector<std::string>& channel_names)
{
  std::vector<std::string> channels;

  switch (pixel_format_) {
    case PixelFormat::Y:
      channels = { "Y" };
      break;
    // case PixelFormat::YA:    channels = { "Y", "A" };               break;
    case PixelFormat::RGB:
      channels = { "R", "G", "B" };
      break;
    case PixelFormat::RGBA:
      channels = { "R", "G", "B", "A" };
      break;
      // case PixelFormat::RGBW:  channels = { "R", "G", "B", "W"};      break;
      // case PixelFormat::RGBAW: channels = { "R", "G", "B", "A", "W"}; break;
      // case PixelFormat::XYZ:   channels = { "X", "Y", "Z"};           break;
      // case PixelFormat::XYZA:  channels = { "X", "Y", "Z", "A"};      break;
      // case PixelFormat::MultiChannel:
      if (channel_names.size() == 0) {
        for (size_t i = 0; i < channel_count; ++i)
          channels.push_back(fmt::format("ch%i", i));
      }
      else {
        std::vector<std::string> channels_sorted = channel_names;
        std::sort(channels_sorted.begin(), channels_sorted.end());
        for (size_t i = 1; i < channels_sorted.size(); ++i) {
          if (channels_sorted[i] == channels_sorted[i - 1])
            Throw("Bitmap::rebuildStruct(): duplicate channel name \"%s\"",
                  channels_sorted[i]);
        }
        for (size_t i = 0; i < channel_count; ++i)
          channels.push_back(channel_names[i]);
      }
      break;
    default:
      Throw("Unknown pixel format!");
  }

  if (channel_count != 0 && channel_count != channels.size())
    Throw("Bitmap::rebuild_struct(): channel count (%i) does not match pixel "
          "format (%s)!",
          channel_count, (uint32_t) pixel_format_);

  struct_ = std::make_unique<Struct>();
  for (auto ch : channels) {
    bool     is_alpha = ch == "A" && pixel_format_ != PixelFormat::MultiChannel;
    uint32_t flags    = (uint32_t) Struct::Flags::Empty;
    if (!is_alpha && ch != "W" && srgb_gamma_)
      flags |= (uint32_t) Struct::Flags::Gamma;
    if (!is_alpha && ch != "W" && premultiplied_alpha_)
      flags |= +(uint32_t) Struct::Flags::PremultipliedAlpha;
    if (is_alpha)
      flags |= +(uint32_t) Struct::Flags::Alpha;
    if (ch == "W")
      flags |= (uint32_t) Struct::Flags::Weight;
    if (Struct::isInteger(component_format_))
      flags |= (uint32_t) Struct::Flags::Normalized;
    struct_->append(ch, component_format_, flags);
  }
}

size_t Bitmap::bufferSize() const { return pixelCount() * bytesPerPixel(); }

size_t Bitmap::bytesPerPixel() const
{
  size_t result;
  switch (component_format_) {
    case Struct::Type::Int8:
    case Struct::Type::UInt8:
      result = 1;
      break;
    case Struct::Type::Int16:
    case Struct::Type::UInt16:
      result = 2;
      break;
    case Struct::Type::Int32:
    case Struct::Type::UInt32:
      result = 4;
      break;
    case Struct::Type::Int64:
    case Struct::Type::UInt64:
      result = 8;
      break;
    case Struct::Type::Float16:
      result = 2;
      break;
    case Struct::Type::Float32:
      result = 4;
      break;
    case Struct::Type::Float64:
      result = 8;
      break;
    default:
      throw std::runtime_error("Unknown component format: " +
                               std::to_string((uint32_t) component_format_));
  }
  return result * channelCount();
}

void Bitmap::read(Stream* stream, FileFormat format)
{
  if (format == FileFormat::Auto)
    format = detectFileFormat(stream);

  switch (format) {
    // case FileFormat::BMP:
    //   read_bmp(stream);
    //   break;
    case FileFormat::JPEG:
      readJpeg(stream);
      break;
    // case FileFormat::OpenEXR:
    //   read_exr(stream);
    //   break;
    // case FileFormat::RGBE:
    //   read_rgbe(stream);
    //   break;
    // case FileFormat::PFM:
    //   read_pfm(stream);
    //   break;
    // case FileFormat::PPM:
    //   read_ppm(stream);
    //   break;
    // case FileFormat::TGA:
    //   read_tga(stream);
    //   break;
    case FileFormat::PNG:
      readPng(stream);
      break;
    default:
      Throw("Bitmap: Unknown file format!");
  }
}

Bitmap::FileFormat Bitmap::detectFileFormat(Stream* stream)
{
  FileFormat format = FileFormat::Unknown;

  // Try to automatically detect the file format
  size_t  pos = stream->tell();
  uint8_t start[8];
  stream->read(start, 8);

  //  if (start[0] == 'B' && start[1] == 'M') {
  //    format = FileFormat::BMP;
  //  }
  //  else if (start[0] == '#' && start[1] == '?') {
  //    format = FileFormat::RGBE;
  //  }
  //  else if (start[0] == 'P' && (start[1] == 'F' || start[1] == 'f')) {
  //    format = FileFormat::PFM;
  //  }
  //  else if (start[0] == 'P' && start[1] == '6') {
  //    format = FileFormat::PPM;
  //  }
  if (start[0] == 0xFF && start[1] == 0xD8) {
    format = FileFormat::JPEG;
  }
  else if (png_sig_cmp(start, 0, 8) == 0) {
    format = FileFormat::PNG;
  }
  //  else if (Imf::isImfMagic((const char*) start)) {
  //    format = FileFormat::OpenEXR;
  //  }
  //  else {
  //    // Check for a TGAv2 file
  //    char footer[18];
  //    stream->seek(stream->size() - 18);
  //    stream->read(footer, 18);
  //    if (footer[17] == 0 && strncmp(footer, "TRUEVISION-XFILE.", 17) == 0)
  //      format = FileFormat::TGA;
  //  }
  stream->seek(pos);
  return format;
}

// -----------------------------------------------------------------------------
// Bitmap JPEG I/O
// -----------------------------------------------------------------------------

extern "C" {
static const size_t jpeg_buffer_size = 0x8000;

typedef struct {
  struct jpeg_source_mgr mgr;
  JOCTET*                buffer;
  Stream*                stream;
} jbuf_in_t;

typedef struct {
  struct jpeg_destination_mgr mgr;
  JOCTET*                     buffer;
  Stream*                     stream;
} jbuf_out_t;

METHODDEF(void) jpegInitSource(j_decompress_ptr cinfo)
{
  jbuf_in_t* p = (jbuf_in_t*) cinfo->src;
  p->buffer    = new JOCTET[jpeg_buffer_size];
}

METHODDEF(boolean) jpegFillInputBuffer(j_decompress_ptr cinfo)
{
  jbuf_in_t* p          = (jbuf_in_t*) cinfo->src;
  size_t     bytes_read = 0;

  try {
    p->stream->read(p->buffer, jpeg_buffer_size);
    bytes_read = jpeg_buffer_size;
  }
  catch (const EOFException& e) {
    bytes_read = e.gcount();
    if (bytes_read == 0) {
      // Insert a fake EOI marker
      p->buffer[0] = (JOCTET) 0xFF;
      p->buffer[1] = (JOCTET) JPEG_EOI;
      bytes_read   = 2;
    }
  }

  cinfo->src->bytes_in_buffer = bytes_read;
  cinfo->src->next_input_byte = p->buffer;
  return TRUE;
}

METHODDEF(void) jpegSkipInputData(j_decompress_ptr cinfo, long num_bytes)
{
  if (num_bytes > 0) {
    while (num_bytes > (long) cinfo->src->bytes_in_buffer) {
      num_bytes -= (long) cinfo->src->bytes_in_buffer;
      jpegFillInputBuffer(cinfo);
    }
    cinfo->src->next_input_byte += (size_t) num_bytes;
    cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
  }
}

METHODDEF(void) jpegTermSource(j_decompress_ptr cinfo)
{
  jbuf_in_t* p = (jbuf_in_t*) cinfo->src;
  delete[] p->buffer;
}

METHODDEF(void) jpegInitDestination(j_compress_ptr cinfo)
{
  jbuf_out_t* p = (jbuf_out_t*) cinfo->dest;

  p->buffer               = new JOCTET[jpeg_buffer_size];
  p->mgr.next_output_byte = p->buffer;
  p->mgr.free_in_buffer   = jpeg_buffer_size;
}

METHODDEF(boolean) jpegEmptyOutputBuffer(j_compress_ptr cinfo)
{
  jbuf_out_t* p = (jbuf_out_t*) cinfo->dest;
  p->stream->write(p->buffer, jpeg_buffer_size);
  p->mgr.next_output_byte = p->buffer;
  p->mgr.free_in_buffer   = jpeg_buffer_size;
  return TRUE;
}

METHODDEF(void) jpegTermDestination(j_compress_ptr cinfo)
{
  jbuf_out_t* p = (jbuf_out_t*) cinfo->dest;
  p->stream->write(p->buffer, jpeg_buffer_size - p->mgr.free_in_buffer);
  delete[] p->buffer;
  p->mgr.free_in_buffer = 0;
}

METHODDEF(void) jpegErrorExit(j_common_ptr cinfo)
{
  char msg[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message)(cinfo, msg);
  Throw("Critical libjpeg error: %s", msg);
}
};

void Bitmap::readJpeg(Stream* stream)
{
  // ScopedPhase                   phase(ProfilerPhase::BitmapRead);
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr         jerr;
  jbuf_in_t                     jbuf;

  memset(&jbuf, 0, sizeof(jbuf_in_t));

  cinfo.err       = jpeg_std_error(&jerr);
  jerr.error_exit = jpegErrorExit;
  jpeg_create_decompress(&cinfo);
  cinfo.src                  = (struct jpeg_source_mgr*) &jbuf;
  jbuf.mgr.init_source       = jpegInitSource;
  jbuf.mgr.fill_input_buffer = jpegFillInputBuffer;
  jbuf.mgr.skip_input_data   = jpegSkipInputData;
  jbuf.mgr.term_source       = jpegTermSource;
  jbuf.mgr.resync_to_restart = jpeg_resync_to_restart;
  jbuf.stream                = stream;

  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  size_                = Vec2u(cinfo.output_width, cinfo.output_height);
  component_format_    = Struct::Type::UInt8;
  srgb_gamma_          = true;
  premultiplied_alpha_ = false;

  switch (cinfo.output_components) {
    case 1:
      pixel_format_ = PixelFormat::Y;
      break;
    case 3:
      pixel_format_ = PixelFormat::RGB;
      break;
    case 4:
      pixel_format_ = PixelFormat::RGBA;
      break;
    default:
      Throw("readJpeg(): Unexpected number of components!");
  }

  rebuildStruct();

  auto fs = dynamic_cast<FileStream*>(stream);
  log_->debug("Loading JPEG file \"{}\" ({}x{}, {}, {}) ..",
              fs ? fs->path().string() : "<stream>", size_.x, size_.y,
              pixel_format_, component_format_);

  size_t row_stride =
    (size_t) cinfo.output_width * (size_t) cinfo.output_components;

  data_      = std::make_unique<uint8_t[]>(bufferSize());
  owns_data_ = true;

  auto scanlines = std::make_unique<uint8_t*[]>(size_.y);

  for (size_t i = 0; i < size_.y; ++i)
    scanlines[i] = uint8Data() + row_stride * i;

  // Process scanline by scanline
  int counter = 0;
  while (cinfo.output_scanline < cinfo.output_height)
    counter +=
      jpeg_read_scanlines(&cinfo, scanlines.get() + counter,
                          (JDIMENSION) (size_.y - cinfo.output_scanline));

  // Release the libjpeg data structures
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
}

/*
void Bitmap::write_jpeg(Stream* stream, int quality) const
{
  ScopedPhase                 phase(ProfilerPhase::BitmapWrite);
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr       jerr;
  jbuf_out_t                  jbuf;

  int components = 0;
  if (m_pixel_format == PixelFormat::Y)
    components = 1;
  else if (m_pixel_format == PixelFormat::RGB ||
           m_pixel_format == PixelFormat::XYZ)
    components = 3;
  else
    Throw("write_jpeg(): Unsupported pixel format!");

  if (m_component_format != Struct::Type::UInt8)
    Throw("write_jpeg(): Unsupported component format %s, expected %s",
          m_component_format, Struct::Type::UInt8);

  memset(&jbuf, 0, sizeof(jbuf_out_t));
  cinfo.err       = jpeg_std_error(&jerr);
  jerr.error_exit = jpegErrorExit;
  jpeg_create_compress(&cinfo);

  cinfo.dest                   = (struct jpeg_destination_mgr*) &jbuf;
  jbuf.mgr.init_destination    = jpegInitDestination;
  jbuf.mgr.empty_output_buffer = jpegEmptyOutputBuffer;
  jbuf.mgr.term_destination    = jpegTermDestination;
  jbuf.stream                  = stream;

  cinfo.image_width      = (JDIMENSION) m_size.x();
  cinfo.image_height     = (JDIMENSION) m_size.y();
  cinfo.input_components = components;
  cinfo.in_color_space   = components == 1 ? JCS_GRAYSCALE : JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE);

  if (quality == 100) {
    // Disable chroma subsampling
    cinfo.comp_info[0].v_samp_factor = 1;
    cinfo.comp_info[0].h_samp_factor = 1;
  }

  jpeg_start_compress(&cinfo, TRUE);

  // Write scanline by scanline
  for (size_t i = 0; i < m_size.y(); ++i) {
    uint8_t* source = m_data.get() + i * m_size.x() * cinfo.input_components;
    jpeg_write_scanlines(&cinfo, &source, 1);
  }

  // Release the libjpeg data structures
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
}
*/

// -----------------------------------------------------------------------------
// Bitmap PNG I/O
// -----------------------------------------------------------------------------

static void pngFlushData(png_structp png_ptr)
{
  png_voidp flush_io_ptr = png_get_io_ptr(png_ptr);
  ((Stream*) flush_io_ptr)->flush();
}

static void pngReadData(png_structp png_ptr, png_bytep data, png_size_t length)
{
  png_voidp read_io_ptr = png_get_io_ptr(png_ptr);
  ((Stream*) read_io_ptr)->read(data, length);
}

static void pngWriteData(png_structp png_ptr, png_bytep data, png_size_t length)
{
  png_voidp write_io_ptr = png_get_io_ptr(png_ptr);
  ((Stream*) write_io_ptr)->write(data, length);
}

static void pngErrorFunc(png_structp, png_const_charp msg)
{
  Throw("Fatal libpng error: %s\n", msg);
}

static void pngWarnFunc(png_structp, png_const_charp msg)
{
  if (strstr(msg, "iCCP: known incorrect sRGB profile") != nullptr)
    return;
  spdlog::warn("libpng warning: {}", msg);
}

void Bitmap::readPng(Stream* stream)
{
  png_bytepp rows = nullptr;

  // Create buffers
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                               &pngErrorFunc, &pngWarnFunc);
  if (png_ptr == nullptr)
    Throw("readPng(): Unable to create PNG data structure");

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr) {
    png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    Throw("readPng(): Unable to create PNG information structure");
  }

  // Error handling
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    delete[] rows;
    Throw("readPng(): Error reading the PNG file!");
  }

  // Set read helper function
  png_set_read_fn(png_ptr, stream, (png_rw_ptr) pngReadData);

  int bit_depth, color_type, interlace_type, compression_type, filter_type;
  png_read_info(png_ptr, info_ptr);
  png_uint_32 width = 0, height = 0;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
               &interlace_type, &compression_type, &filter_type);

  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png_ptr); // Expand 1-, 2- and 4-bit
                                             // grayscale

  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png_ptr); // Always expand indexed files

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png_ptr); // Expand transparency to a proper alpha
                                    // channel

// Request various transformations from libpng as necessary
#if defined(LITTLE_ENDIAN)
  if (bit_depth == 16)
    png_set_swap(png_ptr); // Swap the byte order on little endian machines
#endif

  png_set_interlace_handling(png_ptr);

  // Update the information based on the transformations
  png_read_update_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
               &interlace_type, &compression_type, &filter_type);
  size_ = Vec2u(width, height);

  switch (color_type) {
    case PNG_COLOR_TYPE_GRAY:
      pixel_format_ = PixelFormat::Y;
      break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      pixel_format_ = PixelFormat::YA;
      break;
    case PNG_COLOR_TYPE_RGB:
      pixel_format_ = PixelFormat::RGB;
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      pixel_format_ = PixelFormat::RGBA;
      break;
    default:
      Throw("readPng(): Unknown color type %i", color_type);
      break;
  }

  switch (bit_depth) {
    case 8:
      component_format_ = Struct::Type::UInt8;
      break;
    case 16:
      component_format_ = Struct::Type::UInt16;
      break;
    default:
      Throw("readPng(): Unsupported bit depth: %i", bit_depth);
  }

  srgb_gamma_          = true;
  premultiplied_alpha_ = false;

  rebuildStruct();

  // Load any string-valued metadata
  int       text_idx = 0;
  png_textp text_ptr;
  png_get_text(png_ptr, info_ptr, &text_ptr, &text_idx);

  // TODO: metadata not implemented yet
  // for (int i = 0; i < text_idx; ++i, text_ptr++)
  //  metadata_.set_string(text_ptr->key, text_ptr->text);

  auto fs = dynamic_cast<FileStream*>(stream);
  log_->debug("Loading PNG file \"{}\" ({}x{}, {}, {}) ..",
              fs ? fs->path().string() : "<stream>", size_.x, size_.y,
              pixel_format_, component_format_);

  size_t size = bufferSize();
  data_       = std::make_unique<uint8_t[]>(size);
  owns_data_  = true;

  rows             = new png_bytep[size_.y];
  size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
  assert(row_bytes == size / size_.y);

  for (size_t i = 0; i < size_.y; i++)
    rows[i] = uint8Data() + i * row_bytes;

  png_read_image(png_ptr, rows);
  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

  delete[] rows;
}

void Bitmap::rgbToRgba()
{
  if (pixel_format_ != PixelFormat::RGB)
    Throw("convertToRGBA(): Unexpected pixel format ({})", pixel_format_);

  pixel_format_ = PixelFormat::RGBA;
  rebuildStruct();

  if (owns_data_) {
    size_t                     size   = bufferSize();
    std::unique_ptr<uint8_t[]> tmp    = std::make_unique<uint8_t[]>(size);
    uint8_t*                   p_tmp  = tmp.get();
    uint8_t*                   p_data = uint8Data();

    // TODO: This may need to be optimized for high resolution textures.
    // A 1024x1024 texture was converted in about a millisecond on my laptop.
    const uint32_t rgb_channels = 3;
    for (uint32_t i = 0; i < size_.y; ++i) {
      uint32_t row_i = i * size_.x;
      for (uint32_t j = 0; j < size_.x; ++j) {
        uint32_t index        = row_i + j;
        uint32_t rgb_index    = rgb_channels * index;
        uint32_t rgba_index   = rgb_index + index;
        p_tmp[rgba_index]     = p_data[rgb_index];
        p_tmp[rgba_index + 1] = p_data[rgb_index + 1];
        p_tmp[rgba_index + 2] = p_data[rgb_index + 2];
        p_tmp[rgba_index + 3] = 0xff;
      }
    }
    data_ = std::move(tmp);
  }
}

std::ostream& operator<<(std::ostream& os, const Bitmap::PixelFormat& value)
{
  switch (value) {
    case Bitmap::PixelFormat::Y:
      os << "y";
      break;
    case Bitmap::PixelFormat::YA:
      os << "ya";
      break;
    case Bitmap::PixelFormat::RGB:
      os << "rgb";
      break;
    case Bitmap::PixelFormat::RGBA:
      os << "rgba";
      break;
    case Bitmap::PixelFormat::RGBW:
      os << "rgbw";
      break;
    case Bitmap::PixelFormat::RGBAW:
      os << "rgbaw";
      break;
    case Bitmap::PixelFormat::XYZ:
      os << "xyz";
      break;
    case Bitmap::PixelFormat::XYZA:
      os << "xyza";
      break;
    case Bitmap::PixelFormat::MultiChannel:
      os << "multichannel";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Bitmap::FileFormat& value)
{
  switch (value) {
    case Bitmap::FileFormat::PNG:
      os << "PNG";
      break;
    case Bitmap::FileFormat::OpenEXR:
      os << "OpenEXR";
      break;
    case Bitmap::FileFormat::JPEG:
      os << "JPEG";
      break;
    case Bitmap::FileFormat::BMP:
      os << "BMP";
      break;
    case Bitmap::FileFormat::PFM:
      os << "PFM";
      break;
    case Bitmap::FileFormat::PPM:
      os << "PPM";
      break;
    case Bitmap::FileFormat::RGBE:
      os << "RGBE";
      break;
    case Bitmap::FileFormat::Auto:
      os << "Auto";
      break;
    default:
      Throw("Unknown file format!");
  }
  return os;
}
}; // namespace eldr
