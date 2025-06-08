app_version_numbers    = meson.project_version().split('.')
app_version_variant    = app_version_numbers[0]
app_version_major      = app_version_numbers[1]
app_version_minor      = app_version_numbers[2]
app_version_patch      = app_version_numbers[3]
engine_version_numbers = get_option('engine_version').split('.')
engine_version_variant = engine_version_numbers[0]
engine_version_major   = engine_version_numbers[1]
engine_version_minor   = engine_version_numbers[2]
engine_version_patch   = engine_version_numbers[3]

conf_data = configuration_data()
conf_data.set('EL_AUTHORS', 'Johannes Nilsson')
conf_data.set('EL_YEAR', get_option('build_year'))
conf_data.set('EL_NAME', meson.project_name())
conf_data.set('EL_APP_VERSION_VARIANT', app_version_variant)
conf_data.set('EL_APP_VERSION_MAJOR', app_version_major)
conf_data.set('EL_APP_VERSION_MINOR', app_version_minor)
conf_data.set('EL_APP_VERSION_PATCH', app_version_patch)
conf_data.set('EL_ENGINE_NAME', 'Eldr vulkan backend')
conf_data.set('EL_ENGINE_VERSION_VARIANT', engine_version_variant)
conf_data.set('EL_ENGINE_VERSION_MAJOR', engine_version_major)
conf_data.set('EL_ENGINE_VERSION_MINOR', engine_version_minor)
conf_data.set('EL_ENGINE_VERSION_PATCH', engine_version_patch)
conf_data.set('MESON_BUILD_TYPE', get_option('buildtype'))

# TODO: get this to work
# vcs_tag(command: ['git', 'rev-parse', '--short', 'HEAD'], 
#       input:'gitinfo.hpp.in',
#       output:'gitinfo.hpp',
#       install: true,
#       replace_string:'@GIT_HASH@')

configure_file(
  input: 'buildinfo.hpp.in',
  output: 'buildinfo.hpp',
  configuration: conf_data,
)

