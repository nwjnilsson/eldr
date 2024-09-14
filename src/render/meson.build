src = ['scene.cpp', 'mesh.cpp']
eldr_render = library(
  'eldr-render',
  sources : src,
  include_directories : [include, tinyobjloader_inc],
  dependencies : [libglfw, libvulkan, libspdlog],
  link_with : [eldr_core])
  
