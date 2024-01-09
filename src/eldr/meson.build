src = ['main.cpp', 'eldr-app.cpp']
executable('eldr',
  sources : src,
  include_directories : [include, inc_cxxopts, inc_imgui],
  dependencies : [libglfw, libvulkan],
  link_with : [eldr_gui, eldr_render])
