src = ['gui.cpp', 'vulkan-backend.cpp']
eldr_gui = static_library(
  'eldr-gui',
  sources : src,
  include_directories : [include, inc_imgui],
  dependencies : [libglfw, libvulkan],
  link_with : [imgui])
