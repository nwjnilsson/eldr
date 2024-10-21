src = ['scene.cpp', 'mesh.cpp']
eldr_render = library(
  'eldr-render',
  sources : src,
  include_directories : [eldr_headers, tinyobjloader_headers],
  dependencies : [glfw_dep, vulkan_dep, spdlog_dep],
  link_with : [eldr_core])
  
