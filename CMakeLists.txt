cmake_minimum_required(VERSION 3.4)

project(spleeter++)

set(CMAKE_CXX_STANDARD 14)

option(spleeter_enable_tests "Enable unit tests" ON)
set(TENSORFLOW_CC_INSTALL_DIR "" CACHE STRING "The tensorflow_cc install directory")

# External dependencies
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(add_spleeter)
include(add_tensorflow)
include(export_spleeter_models)
# --
#
# include_directories(src)
# add_subdirectory(src)
#
if (${spleeter_enable_tests})
  set(test_audio_path ${CMAKE_CURRENT_BINARY_DIR}/test.wav)
  if (NOT EXISTS ${test_audio_path})
    file(DOWNLOAD "https://archive.org/download/test_wav/Untitled3.wav" "${test_audio_path}")
  endif()
  include(add_googletest)
  include(add_wave)
  add_subdirectory(test)
endif()
