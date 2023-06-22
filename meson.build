project('neosql-core', 'c',
  version: '0.0.1',
  default_options: [
    'c_std=c2x',
    'buildtype=debug',
    'optimization=2',
    'b_lundef=false',
    'strip=true',
    'prefix=/tmp/install'
])

subdir('src')

cc = meson.get_compiler('c')
warning_flags = [
  '-Weverything',
  '-Wfloat-equal',
  '-Wundef',
  '-Wshadow',
  '-Wpointer-arith',
  '-Wcast-align',
  '-Wwrite-strings',
  '-Wcast-qual',
  '-Wswitch-default',
  '-Wswitch-enum',
  '-Wconversion',
  '-Wformat=2',
  '-Wpedantic',
  '-Wformat-security',
  '-Wtype-limits',
  '-Wunused-result',
  '-Wvariadic-macros',
  '-Wno-format-extra-args',
  '-Wno-missing-prototypes',
  '-Wno-padded',
  '-Wno-missing-declarations',
  '-Wno-declaration-after-statement',
]

sanitizer_flags = [
  '-fsanitize=address,undefined',  
  '-fno-omit-frame-pointer',
]

incdir = include_directories('include')
neosql_core = shared_library('neosql_core', 
                             neosql_core_src, 
                             c_args: cc.get_supported_arguments(warning_flags),
                             include_directories: incdir, 
                             install: true,
                             name_prefix: '_')

if get_option('buildtype') != 'release'
  libcriterion = subproject('libcriterion')
  subdir('tests')
endif
