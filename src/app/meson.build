src = [
  'app.cpp',
  'window.cpp',
  'keyboardmouseinput.cpp',
  'scenemanager.cpp'
  ]

lib_app = static_library('lib-app',
  sources : src,
  include_directories : [eldr_headers],
  dependencies : [
    glfw_dep,
    imgui_dep,
    fastgltf_dep,
    ],
  link_with : [lib_engine, lib_render])
