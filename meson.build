# Note about versioning: Vulkan uses a four component version number, where the
# first component is a "variant". For consistency I decided to go with a four
# component app version as well, even though the first component will most
# likely always be 0 (but you never know)
project('Eldr', 'cpp',
  version : '0.0.0.1',
  license : 'MIT',
  meson_version : '>=1.1.0',
  default_options : [
    'cpp_std=c++23',
    'buildtype=debug',
    'warning_level=2',    # -Wall -Wextra
    'b_ndebug=if-release'
    ]
  )

add_global_arguments(
  #'-fmodules-ts',
    language : 'cpp'
    )

# Meson will use -O3 if buildtype=release
build_type = get_option('buildtype')
if build_type.startswith('debugoptimized')
  add_project_arguments(
    '-DDEBUG',
    language : 'cpp'
    )
elif build_type.startswith('debug')
  add_project_arguments(
    '-DDEBUG',
    '-DELDR_VULKAN_DEBUG_REPORT',
    language : 'cpp'
    )
elif build_type.startswith('release')
  add_project_arguments(
    '-DNDEBUG',
    language : 'cpp'
    )
endif

log_level = get_option('log_level')
add_project_arguments('-DLOG_ACTIVE_LEVEL_' + log_level.to_upper(), language : 'cpp')


# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
vulkan_dep       = dependency('vulkan',  version: '>=1.2.131')

vma_proj         = subproject('vulkan-memory-allocator',
  default_options: 'warning_level=0')
vma_dep          = vma_proj.get_variable('vma_allocator_dep')

libjpeg_dep      = dependency('libturbojpeg',
  required: true,
  default_options: 'warning_level=0')

libpng_dep       = dependency('libpng',
  default_options: 'warning_level=0')

# libxml2_dep = dependency('libxml-2.0',
#   default_options: ['warning_level=0', 'c_std=c18'])

glfw_dep         = dependency('glfw3',
  default_options: 'warning_level=0')

imgui_dep        = dependency('imgui',
  default_options: 'warning_level=0')

glm_dep          = dependency('glm',
  default_options: 'warning_level=0')

cxxopts_dep      = dependency('cxxopts',
  default_options: 'warning_level=0')

# fastgltf and rapidobj use CMake and there are no meson.build patch files for
# these projects. The workaround is to write one's own patch file and store it
# somewhere, or use the CMake module in Meson like I do below. This practically
# means using two build systems though and seems kind of flaky.
cmake = import('cmake')
fastgltf_opts = cmake.subproject_options()
fastgltf_opts.add_cmake_defines({'FASTGLTF_COMPILE_AS_CPP20': true})
fastgltf_opts.append_compile_args('cpp', '-w')
fastgltf_dep = cmake.subproject(
  'fastgltf', options: fastgltf_opts
  ).dependency('fastgltf')

rapidobj_dep= cmake.subproject('rapidobj').dependency('rapidobj')

# ------------------------------------------------------------------------------
# The rest
# ------------------------------------------------------------------------------
eldr_include = include_directories('src')

subdir('ext')
subdir('src')
