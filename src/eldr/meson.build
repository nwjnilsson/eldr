libglfw = dependency('glfw3')
executable('eldr',
  'eldr.cpp',
  include_directories : [inc, cxxopts],
  dependencies : [libglfw],
  link_with : eldr_gui)
