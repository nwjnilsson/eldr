project('eldr', 'cpp',
  version : '0.0.1',
  license : 'MIT',
  default_options : [
    'cpp_std=c++20',
    'buildtype=debug',
    'warning_level=2',    # -Wall -Wextra
    'b_ndebug=if-release'
    ]
  )

add_global_arguments(
    #'-fmodules-ts',       # Needed to get module support in GCC (didn't work, try in future)
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

# Make sure the order of these are right
subdir('external')
include = include_directories('include')
subdir('src')
