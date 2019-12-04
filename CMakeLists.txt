cmake_minimum_required(VERSION 3.4)

project(spleeter++)

option(spleeter_enable_tests "Enable unit tests" ON)
option(spleeter_build_examples "Enable examples" OFF)

# External dependencies
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(add_spleeter)
# Force the use of the created environment in pybind11
set(PYTHON_EXECUTABLE ${spleeter_env_dir}/bin/python CACHE STRING "")
include(add_pybind11)
# --

include_directories(src)
add_subdirectory(src)

if (${spleeter_enable_tests})
  include(add_googletest)
  add_subdirectory(test)
endif()
