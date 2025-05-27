src = [
  'app.cpp',
  'window.cpp',
  'keyboardmouseinput.cpp'
  ]

lib_app = static_library('lib-app',
  sources : src,
  include_directories : [eldr_headers],
  dependencies : [
    glfw_dep,
    vulkan_dep,
    vma_dep,
    imgui_dep,
    cxxopts_dep
    ],
  link_with : [lib_engine, lib_render])
