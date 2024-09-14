src = ['main.cpp', 'app.cpp']
executable('eldr',
  sources : src,
  include_directories : [include, cxxopts_inc, imgui_inc],
  dependencies : [libglfw, libvulkan, libspdlog],
  link_with : [imgui, vulkan_engine, eldr_render])
