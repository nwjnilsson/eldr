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
glfw_dep      = dependency('glfw3',   version: '>=3.3.2')
vulkan_dep    = dependency('vulkan',  version: '>=1.2.131')
libjpeg_dep   = dependency('libjpeg', version: '>=2.0.3')
libpng_dep    = dependency('libpng',  version: '>=1.6.3')
spdlog_dep    = dependency('spdlog',  version: '>=1.5.0')
vma_proj      = subproject('vulkan-memory-allocator')
vma_dep       = vma_proj.get_variable('vma_allocator_dep')
imgui_proj    = subproject('imgui')
imgui_dep     = imgui_proj.get_variable('imgui_dep')
cxxopts_proj  = subproject('cxxopts')
cxxopts_dep   = cxxopts_proj.get_variable('cxxopts_dep')

# ------------------------------------------------------------------------------
# The rest
# ------------------------------------------------------------------------------
eldr_headers = include_directories('include')

subdir('external')
subdir('src')
