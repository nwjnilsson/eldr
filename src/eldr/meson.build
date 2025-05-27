src = [
  'eldr.cpp',
  ]

executable('eldr',
  sources : src,
  include_directories : [eldr_headers],
  dependencies : [cxxopts_dep],
  link_with : [lib_engine, lib_render, lib_app])
