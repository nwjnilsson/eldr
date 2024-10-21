src = ['main.cpp', 'app.cpp']

executable('eldr',
  sources : src,
  include_directories : [eldr_headers, imgui_headers],
  dependencies : [glfw_dep, vulkan_dep, spdlog_dep, cxxopts_dep],
  link_with : [imgui, vulkan_engine, eldr_render])
