project('neosql-core', 'c',
  version: '0.0.1',
  default_options: [
    'warning_level=everything',
    'c_std=c2x',
    'buildtype=debug',
    # 'b_sanitize=address,undefined',
    'optimization=2',
    'b_lundef=false'
])

subdir('core')

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
]


add_project_arguments(cc.get_supported_arguments(warning_flags), language: 'c')

incdir = include_directories('core/include')
neosql_core = shared_library('neosql-core', neosql_core_src, include_directories: incdir)

libcriterion = subproject('libcriterion')
subdir('tests/core_tests')
