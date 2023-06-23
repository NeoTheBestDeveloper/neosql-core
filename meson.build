project('neosql-core', 'c',
  version: '0.0.1',
  meson_version: '>=1.0.0',
  license: 'MIT',
  default_options: [
    'warning_level=everything',
    'c_std=c2x',
    'buildtype=debug',
    'optimization=2',
    'b_lundef=false',
    'strip=true',
])

subdir('src')

cc = meson.get_compiler('c')
warning_flags = [
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
  '-Wno-suggest-attribute=const',
]

add_project_arguments(cc.get_supported_arguments(warning_flags), language: 'c')


incdir = include_directories('include')
neosql_core = shared_library('neosql_core', 
                             neosql_core_src, 
                             include_directories: incdir, 
                             install: true,
                             name_prefix: '_')

if get_option('buildtype') != 'release'
  libcriterion = subproject('libcriterion')
  subdir('tests')
endif
