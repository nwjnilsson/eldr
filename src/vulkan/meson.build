src = [
  'vktools/format.cpp',
  'wrappers/commandbuffer.cpp',
  'wrappers/commandpool.cpp',
  'wrappers/debugutilsmessenger.cpp',
  'wrappers/descriptorpool.cpp',
  'wrappers/descriptorsetlayout.cpp',
  'wrappers/device.cpp',
  'wrappers/fence.cpp',
  'wrappers/framebuffer.cpp',
  'wrappers/image.cpp',
  'wrappers/instance.cpp',
  'wrappers/pipeline.cpp',
  'wrappers/sampler.cpp',
  'wrappers/semaphore.cpp',
  'wrappers/shader.cpp',
  'wrappers/surface.cpp',
  'wrappers/swapchain.cpp',
  'wrappers/texture.cpp',
  'descriptorallocator.cpp',
  'descriptorsetlayoutbuilder.cpp',
  'descriptorwriter.cpp',
  'engine.cpp',
  'imgui.cpp',
  'material.cpp',
  'pipelinebuilder.cpp',
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
  
