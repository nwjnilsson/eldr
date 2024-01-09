src = ['gui.cpp', 'vulkan-wrapper.cpp']
eldr_gui = static_library(
  'eldr-gui',
  sources : src,
  include_directories : [include, inc_imgui],
  dependencies : [libglfw, libvulkan],
  link_with : [imgui])
