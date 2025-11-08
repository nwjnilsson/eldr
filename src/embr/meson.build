sys = host_machine.system()
cpuinfo = ''
avx = false
avx2 = false
avx512 = false

if sys == 'linux'
  # Works on Linux
  cpuinfo = run_command('grep', '-m1', 'flags', '/proc/cpuinfo', check: false).stdout()
elif sys == 'darwin'
  # macOS: use sysctl
  cpuinfo = run_command('sysctl', '-a', check: false).stdout()
elif sys == 'windows'
  # Windows: PowerShell query
  cpuinfo = run_command('powershell', '-Command',
                        'Get-CimInstance Win32_Processor | Select-String "AVX"',
                        check: false).stdout()
else
  message('Unknown OS; CPU feature detection skipped')
endif

cpuinfo = cpuinfo.to_lower()

avx512 = 'avx512' in cpuinfo
avx2 = 'avx2' in cpuinfo
avx = 'avx ' in cpuinfo or 'avx,' in cpuinfo or 'avx\n' in cpuinfo
sse2 = 'sse2' in cpuinfo

if avx512
  message('CPU supports AVX512')
elif avx2
  message('CPU supports AVX2')
elif avx
  message('CPU supports AVX')
elif sse2
  message('CPU supports SSE2')
else
  message('No SIMD support detected')
endif

# Optional: also check compiler support and add flags automatically
cc = meson.get_compiler('cpp')

if avx512 and cc.has_argument('-mavx512f')
  add_project_arguments('-mavx512f', language: 'cpp')
  add_project_arguments('-DELDR_X86_AVX512', language: 'cpp')
  message('Will build with AVX512')
elif avx2 and cc.has_argument('-mavx2')
  add_project_arguments('-mavx2', language: 'cpp')
  add_project_arguments('-DELDR_X86_AVX', language: 'cpp')
  add_project_arguments('-DELDR_X86_AVX_HAS_FMADD', language: 'cpp')
  message('Will build with AVX2')
elif avx and cc.has_argument('-mavx')
  add_project_arguments('-mavx', language: 'cpp')
  add_project_arguments('-DELDR_X86_AVX', language: 'cpp')
  message('Will build with AVX')
elif sse2 and cc.has_argument('-msse2')
  add_project_arguments('-msse2', language: 'cpp')
  add_project_arguments('-DELDR_X86_SSE2', language: 'cpp')
  message('Will build with SSE2')
else
  warning('Will build without SIMD acceleration')
endif

