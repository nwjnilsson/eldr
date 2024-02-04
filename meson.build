project('eldr', 'cpp',
  version : '0.0.1',
  license : 'MIT',
  default_options : [
    'cpp_std=c++20',
    'buildtype=debug',
    'warning_level=2',    # -Wall -Wextra
    'b_ndebug=if-release',
    'default_library=shared'
    ]
  )

add_global_arguments(
    #'-fmodules-ts',       # Needed to get module support in GCC (didn't work, try in future)
    '-DSPDLOG_COMPILED_LIB',
    language : 'cpp'
    )

# Meson will use -O3 if buildtype=release
if get_option('buildtype').startswith('debug')
  add_project_arguments(
    '-DELDR_DEBUG',
    '-DELDR_VULKAN_DEBUG_REPORT',
    language : 'cpp'
    )
  #else
  #  add_project_arguments(
  #    )
endif  

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
libglfw       = dependency('glfw3',   version: '>=3.3.2')
libvulkan     = dependency('vulkan',  version: '>=1.2.131')
libjpeg       = dependency('libjpeg', version: '>=2.0.3')
libspdlog     = dependency('spdlog',  version: '>=1.5.0',
                                      modules: ['spdlog::spdlog'], static: true)

# ------------------------------------------------------------------------------
# The rest
# ------------------------------------------------------------------------------
include = include_directories('include')
subdir('external')
subdir('src')
