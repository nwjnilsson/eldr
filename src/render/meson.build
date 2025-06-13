src = [
  'endpoint.cpp',
  'mesh.cpp',
  'scene.cpp',
  ]
lib_render = static_library(
  'lib-render',
  sources : src,
  include_directories : [eldr_headers, tinyobjloader_headers],
  dependencies : [
    glfw_dep,
    vulkan_dep,
    vma_dep,
    ],
  link_with : [lib_core])
  
