src = [
  'main.cpp',
  'app.cpp',
  'window.cpp',
  'keyboardmouseinput.cpp'
  ]

executable('eldr',
  sources : src,
  include_directories : [eldr_headers],
  dependencies : [
    glfw_dep,
    vulkan_dep,
    vma_dep,
    imgui_dep,
    spdlog_dep,
    cxxopts_dep
    ],
  link_with : [vulkan_engine, eldr_render])
