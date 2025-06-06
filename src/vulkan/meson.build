src = [
  'vktools/format.cpp',
  'wrappers/buffer.cpp',
  'wrappers/commandbuffer.cpp',
  'wrappers/commandpool.cpp',
  'wrappers/debugutilsmessenger.cpp',
  'wrappers/descriptorpool.cpp',
  'wrappers/descriptorsetlayout.cpp',
  'wrappers/device.cpp',
  'wrappers/fence.cpp',
  'wrappers/framebuffer.cpp',
  'wrappers/image.cpp',
  'wrappers/imageview.cpp',
  'wrappers/instance.cpp',
  'wrappers/pipeline.cpp',
  'wrappers/sampler.cpp',
  'wrappers/semaphore.cpp',
  'wrappers/shader.cpp',
  'wrappers/surface.cpp',
  'wrappers/swapchain.cpp',
  'descriptorallocator.cpp',
  'descriptorsetlayoutbuilder.cpp',
  'descriptorwriter.cpp',
  'engine.cpp',
  'imgui.cpp',
  'material.cpp',
  'pipelinebuilder.cpp',
  'rendergraph.cpp',
  ]


lib_engine = static_library(
  'lib-engine',
  sources : src,
  include_directories : [eldr_headers],
  dependencies : [fmt_dep, glfw_dep, vulkan_dep, vma_dep, imgui_dep],
  link_with : [lib_core])
  
