project('Eldr', 'cpp',
  version : '0.1.0',
  license : 'MIT',
  default_options : [
    'cpp_std=c++20',
    'buildtype=debug',
    'warning_level=2',    # -Wall -Wextra
    'b_ndebug=if-release',
    'default_library=static'
    ]
  )


add_global_arguments(
    #'-fmodules-ts',       # Needed to get module support in GCC (didn't work, try in future)
    language : 'cpp'
    )

# Meson will use -O3 if buildtype=release
if get_option('buildtype').startswith('debugoptimized')
  add_project_arguments(
    '-D_DEBUG',
    '-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO',
    language : 'cpp'
    )
elif get_option('buildtype').startswith('debug')
  add_project_arguments(
    '-D_DEBUG',
    '-DELDR_VULKAN_DEBUG_REPORT',
    '-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE',
    language : 'cpp'
    )
elif get_option('buildtype').startswith('release')
  add_project_arguments(
    '-DNDEBUG',
    '-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_WARN',
    language : 'cpp'
    )
endif  

# ------------------------------------------------------------------------------
# Vulkan engine conf data
# ------------------------------------------------------------------------------
app_version_numbers = meson.project_version().split('.')

engine_version_variant = '0'
engine_version_major   = '0'
engine_version_minor   = '1'
engine_version_patch   = '0'

vulkan_conf_data = configuration_data()
vulkan_conf_data.set('ELDR_APP_NAME', meson.project_name())
vulkan_conf_data.set('ELDR_APP_VERSION_MAJOR', app_version_numbers[0])
vulkan_conf_data.set('ELDR_APP_VERSION_MINOR', app_version_numbers[1])
vulkan_conf_data.set('ELDR_APP_VERSION_PATCH', app_version_numbers[2])
vulkan_conf_data.set('ELDR_ENGINE_VERSION_VARIANT', engine_version_variant)
vulkan_conf_data.set('ELDR_ENGINE_VERSION_MAJOR', engine_version_major)
vulkan_conf_data.set('ELDR_ENGINE_VERSION_MINOR', engine_version_minor)
vulkan_conf_data.set('ELDR_ENGINE_VERSION_PATCH', engine_version_patch)
vulkan_conf_data.set('ELDR_ENGINE_NAME', 'eldr_pbr_engine')
vulkan_conf_data.set('MESON_BUILD_TYPE', get_option('buildtype'))

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
# Dependencies
vma_proj         = subproject('vulkan-memory-allocator',
  default_options: 'warning_level=0')
vma_dep          = vma_proj.get_variable('vma_allocator_dep')
vulkan_dep       = dependency('vulkan',  version: '>=1.2.131')
libjpeg_dep      = dependency('libturbojpeg',
  fallback: ['libjpeg-turbo', 'libjpeg'],
  default_options: 'warning_level=0')
libpng_dep       = dependency('libpng',
  fallback: ['libpng', 'libpng_dep'],
  default_options: 'warning_level=0')
glfw_dep         = dependency('glfw',
  fallback: ['glfw', 'glfw_dep'],
  default_options: 'warning_level=0')
imgui_dep        = dependency('imgui',
  fallback: ['imgui', 'imgui_dep'],
  default_options: 'warning_level=0')
glm_dep          = dependency('glm',
  fallback: ['glm', 'glm_dep'],
  default_options: 'warning_level=0')
cxxopts_dep      = dependency('cxxopts',
  fallback: ['cxxopts', 'cxxopts_dep'],
  default_options: 'warning_level=0')
spdlog_dep       = dependency('spdlog',
  fallback: ['spdlog', 'spdlog_dep'],
  default_options: 'warning_level=0')
libxkbcommon_dep = dependency('libxkbcommon',
  fallback: ['libxkbcommon', 'dep_libxkbcommon'],
  default_options: 'warning_level=0')

# ------------------------------------------------------------------------------
# The rest
# ------------------------------------------------------------------------------
eldr_headers = include_directories('include')

subdir('external')
subdir('src')
