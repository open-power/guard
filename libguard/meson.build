# SPDX-License-Identifier: Apache-2.0
headers = [
  'guard_interface.hpp',
  'guard_file.hpp',
  'guard_entity.hpp',
  'guard_log.hpp',
  'guard_common.hpp',
  'include/guard_record.hpp'
]

sources = [
  'guard_interface.cpp',
  'guard_file.cpp',
  'guard_log.cpp',
  'guard_entity.cpp'
]

libguard_headers = ['.', '..']

install_headers(
  headers,
  subdir: 'libguard')

libguard = library(
  'guard',
  sources,
  implicit_include_directories: false,
  include_directories: libguard_headers,
  version: meson.project_version(),
  install: true)
