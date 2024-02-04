inc_cxxopts = include_directories('cxxopts/include')

# ------------------------------------------------------------------------------
# Build Dear Imgui
# ------------------------------------------------------------------------------
inc_imgui = include_directories('imgui')
imgui = library(
  'imgui',
  sources : [
    'imgui/imgui.cpp',
    'imgui/imgui_draw.cpp',
    'imgui/imgui_tables.cpp',
    'imgui/imgui_widgets.cpp'
    ],
  include_directories : inc_imgui
  )
