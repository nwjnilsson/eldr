src = ['gui.cpp']
eldr_gui = static_library(
  'eldr-gui',
  sources : src,
  include_directories : [include, inc_imgui],
  dependencies : [libglfw],
  link_with : [imgui])
