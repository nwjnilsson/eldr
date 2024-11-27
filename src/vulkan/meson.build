src = [
  'wrappers/buffer.cpp',
  'wrappers/commandbuffer.cpp',
  'wrappers/commandpool.cpp',
  'wrappers/debugutilsmessenger.cpp',
  'wrappers/descriptor.cpp',
  'wrappers/descriptorbuilder.cpp',
  'wrappers/descriptorpool.cpp',
  'wrappers/descriptorsetlayout.cpp',
  'wrappers/device.cpp',
  'wrappers/fence.cpp',
  'wrappers/framebuffer.cpp',
  'wrappers/gputexture.cpp',
  'wrappers/image.cpp',
  'wrappers/instance.cpp',
  'wrappers/sampler.cpp',
  'wrappers/semaphore.cpp',
  'wrappers/shader.cpp',
  'wrappers/surface.cpp',
  'wrappers/swapchain.cpp',
  'vktools/format.cpp',
  'engine.cpp',
  'imgui.cpp',
  'rendergraph.cpp',
  'vertex.cpp'
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
  
