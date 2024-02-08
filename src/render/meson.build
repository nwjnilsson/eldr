src = ['scene.cpp', 'vulkan-wrapper.cpp']
eldr_render = library(
  'eldr-render',
  sources : src,
  include_directories : [include],
  dependencies : [libglfw, libvulkan, libspdlog],
  link_with : [eldr_core])
  
