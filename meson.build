# SPDX-License-Identifier: Apache-2.0
# Adding "c" in languages list of project api because dependency library
# (libpdbg - to use device tree) implemented in c and meson throwing error
# for that.
# libguard/meson.build:1:0: ERROR: Tried to access compiler for unspecified
# language "c".
project('guard', ['cpp', 'c'],
        version: '0.1', meson_version: '>=0.49.0',
        default_options: [
          'warning_level=3',
          'default_library=shared',
          'werror=true',
          'cpp_std=c++17'
        ])

# Wno-psabi reduces the number of "Note:" messages when cross-compiling some STL
# stuff for ARM. See https://stackoverflow.com/questions/48149323/strange-gcc-warning-when-compiling-qt-project
# Basically, gcc 6 and gcc 7 are not ABI compatible, but since the whole OpenBMC
# project uses the same compiler, we can safely ignmore these info notes.
add_project_arguments('-Wno-psabi', language: 'cpp')

conf_data = configuration_data()

conf_data.set_quoted('GUARD_RO_PATH', get_option('GUARD_RO_PATH'),
                      description : 'GUARD file in pnor ro partition'
                    )
conf_data.set_quoted('GUARD_PRSV_PATH', get_option('GUARD_PRSV_PATH'),
                      description : 'GUARD file in pnor prsv partition'
                    )

conf_data.set('DEV_TREE', get_option('devtree').enabled(),
              description : 'Use device tree to get physical path value'
             )

conf_data.set('VERBOSE_LEVEL', get_option('verbose'),
              description : 'Build time log level for trace')

configure_file(output: 'config.h',
  configuration: conf_data
)

#add_project_arguments('-DPGUARD', language : 'cpp')
add_project_arguments('-DGUARD_VERSION', language : 'cpp')
set_variable('GUARD_VERSION', '1.0')

subdir('libguard')

deps = [
  libguard,
]

executable(
  'guard',
  'guard.cpp',
  implicit_include_directories: true,
  link_with: libguard,
  install: true,
  install_dir: get_option('bindir'))

if get_option('tests').enabled()
  subdir('test')
endif

