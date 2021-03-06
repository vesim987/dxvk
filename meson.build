project('dxvk', ['c', 'cpp'])

cpu_family = target_machine.cpu_family()

dxvk_compiler = meson.get_compiler('cpp')
if dxvk_compiler.get_id() == 'msvc'
  add_global_arguments('-DNOMINMAX', language : 'cpp')
  dxvk_cpp_std='c++latest'
else
  dxvk_cpp_std='c++1z'
endif

dxvk_include_path = include_directories('./include')

if (cpu_family == 'x86_64')
  dxvk_library_path = meson.source_root() + '/lib'
else
  dxvk_library_path = meson.source_root() + '/lib32'
endif

lib_vulkan  = dxvk_compiler.find_library('vulkan-1', dirs : dxvk_library_path)
lib_d3d11   = dxvk_compiler.find_library('d3d11')
lib_dxgi    = dxvk_compiler.find_library('dxgi')

if dxvk_compiler.get_id() != 'msvc'
  lib_d3dcompiler_47 = dxvk_compiler.find_library('d3dcompiler_47')
endif

git_command = find_program('git', required: false)

if git_command.found()
  git = run_command('git', ['-C', meson.source_root(), 'rev-parse', '--short', 'HEAD'])
  if git.returncode() == 0
    add_global_arguments('-DDXVK_VERSION="'+git.stdout().strip()+'"', language: 'cpp')
  else
    add_global_arguments('-DDXVK_VERSION="unknown"', language: 'cpp')
  endif
else
  add_global_arguments('-DDXVK_VERSION="unknown"', language: 'cpp')
endif


glsl_compiler = find_program('glslangValidator')
glsl_generator = generator(glsl_compiler,
  output    : [ '@BASENAME@.h' ],
  arguments : [ '-V', '--vn', '@BASENAME@', '@INPUT@', '-o', '@OUTPUT@' ])

subdir('src')
if dxvk_compiler.get_id() != 'msvc'
  subdir('tests')
  subdir('wine_utils')
endif
