src = [ 'bitmap.cpp',
        'stream.cpp',
        'fstream.cpp',
        'mstream.cpp',
        'dstream.cpp',
        'logger.cpp',
        'struct.cpp']

eldr_core = library(
  'eldr-core',
  sources : src,
  include_directories : [eldr_headers],
  dependencies : [spdlog_dep, libjpeg_dep, libpng_dep])
