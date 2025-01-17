# ~~~
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 Scipp contributors (https://github.com/scipp)
# ~~~
execute_process(
  COMMAND conan export .
  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/.conan-recipes/llnl-units"
                    COMMAND_ECHO STDOUT
)

# Conan dependencies
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
  message(
    STATUS
      "Downloading conan.cmake from https://github.com/conan-io/cmake-conan"
  )
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/conan-io/cmake-conan/0.17.0/conan.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake"
    EXPECTED_HASH
      SHA256=3bef79da16c2e031dc429e1dac87a08b9226418b300ce004cc125a82687baeef
    TLS_VERIFY ON
  )
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})

option(CONAN_TBB "Use TBB from conan" OFF)
if(SKBUILD OR CONAN_TBB)
  # Conda builds install tbb via conda, not conan
  set(CONAN_ONETBB onetbb/2021.3.0)
  # The deploy generator is used to install dependencies into a know location.
  # The RUNTIME_DEPENDENCIES DIRECTORIES install option is then used to find the
  # dependencies. This is actually required only for Windows, since on Linux and
  # OSX cmake finds the files anyway, based on the target's rpath.
  set(CONAN_DEPLOY deploy)
  set(CONAN_RUNTIME_DEPENDENCIES "tbb")
endif()

conan_cmake_configure(
  REQUIRES
  benchmark/1.6.1
  boost/1.79.0
  eigen/3.3.9
  gtest/1.11.0
  LLNL-Units/0.6.0
  pybind11/2.6.2
  ${CONAN_ONETBB}
  OPTIONS
  benchmark:shared=False
  boost:header_only=True
  gtest:shared=False
  LLNL-Units:shared=False
  LLNL-Units:fPIC=True
  LLNL-Units:base_type=uint64_t
  LLNL-Units:namespace=llnl::units
  GENERATORS
  cmake_find_package_multi
  ${CONAN_DEPLOY}
)

conan_cmake_autodetect(conan_settings)
if(DEFINED CMAKE_OSX_ARCHITECTURES)
  if("${CMAKE_OSX_ARCHITECTURES}" STREQUAL "arm64")
    # For Apple M1
    set(conan_settings "${conan_settings};arch=armv8")
    if(SKBUILD)
      # There is no tbb pip package for this architecture yet.
      set(THREADING OFF)
    endif()
  endif()
endif()
conan_cmake_install(
  PATH_OR_REFERENCE ${CMAKE_CURRENT_BINARY_DIR} SETTINGS ${conan_settings}
  BUILD outdated
)
