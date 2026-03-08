cc = meson.get_compiler('cpp')

# Use -march=native to enable all CPU features supported by the build machine
# This automatically enables AVX, AVX2, FMA, BMI, etc. based on CPUID detection
if not get_option('disable_simd')
  if cc.has_argument('-march=native')
    add_project_arguments('-march=native', language: 'cpp')
    message('Building with -march=native (native CPU optimization enabled)')
  else
    warning('Compiler does not support -march=native, building without explicit SIMD flags')
  endif
else
  add_project_arguments('-DEL_DISABLE_SIMD', language: 'cpp')
  message('SIMD vectorization explicitly disabled via disable_simd option')
endif
