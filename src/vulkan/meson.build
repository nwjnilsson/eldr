src = [
  'wrappers/buffer.cpp',
  'wrappers/commandbuffer.cpp',
  'wrappers/commandpool.cpp',
  'wrappers/debugutilsmessenger.cpp',
  'wrappers/descriptorpool.cpp',
  'wrappers/descriptorsetlayout.cpp',
  'wrappers/device.cpp',
  'wrappers/fence.cpp',
  'wrappers/framebuffer.cpp',
  'wrappers/texture.cpp',
  'wrappers/image.cpp',
  'wrappers/instance.cpp',
  'wrappers/sampler.cpp',
  'wrappers/semaphore.cpp',
  'wrappers/shader.cpp',
  'wrappers/surface.cpp',
  'wrappers/swapchain.cpp',
  'descriptorwriter.cpp',
  'descriptorsetlayoutbuilder.cpp',
  'vktools/format.cpp',
  'engine.cpp',
  'imgui.cpp',
  'rendergraph.cpp'
  ]

configure_file(
  input : meson.source_root() + '/include/eldr/vulkan/config.hpp.in',
  output : 'engine_config.hpp',
  configuration : vulkan_conf_data
  )

engine_conf_header = include_directories('.')

vulkan_engine = library(
  'vulkan-engine',
  sources : src,
  include_directories : [eldr_headers, engine_conf_header],
  dependencies : [glfw_dep, vulkan_dep, spdlog_dep, vma_dep, imgui_dep],
  link_with : [eldr_core])
  
