src = [ 'bitmap.cpp',
        'dstream.cpp',
        'formatter.cpp',
        'fstream.cpp',
        'logger.cpp',
        'mstream.cpp',
        'progress.cpp',
        'stopwatch.cpp',
        'stream.cpp',
        'struct.cpp',
        'thread.cpp',
        'util.cpp',
        ]



lib_core = library(
  'lib-core',
  sources : src,
  include_directories : [eldr_include],
  dependencies : [
          libjpeg_dep,
          libpng_dep,
          rapidobj_dep
          ])
