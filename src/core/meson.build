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

# All libs can probably be just static but if anything should be shared, it's
# probably the core types
lib_core = library(
  'lib-core',
  sources : src,
  include_directories : [eldr_headers],
  dependencies : [
          fmt_dep,
          libjpeg_dep,
          libpng_dep,
          rapidobj_dep
          ])
