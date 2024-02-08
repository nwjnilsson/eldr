src = [ 'bitmap.cpp',
        'stream.cpp',
        'fstream.cpp',
        'mstream.cpp',
        'dstream.cpp',
        'struct.cpp']
eldr_core = library(
  'eldr-core',
  sources : src,
  include_directories : [include],
  dependencies : [libspdlog, libjpeg])
