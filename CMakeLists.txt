cmake_minimum_required(VERSION 3.4)

project(spleeter++)

set(CMAKE_CXX_STANDARD 14)

option(spleeter_enable_tests "Enable unit tests" ON)
option(spleeter_regenerate_models "Build the models from deezer/spleeter repository (requires conda)" ON)

# See https://github.com/deezer/spleeter/wiki/3.-Models#audio-parameters
set(spleeter_input_frame_count 64 CACHE STRING "Time length of the input spectrogram segment expressed in short time Fourier transform frames")
set(allowed_frame_count "64;128;256;512")
if (NOT spleeter_input_frame_count IN_LIST allowed_frame_count)
  message(FATAL_ERROR "Can't use ${spleeter_input_frame_count} as spleeter_input_frame_count. Only 64/128/256/512 are supported")
endif()
message(STATUS "using spleeter_input_frame_count ${spleeter_input_frame_count}")

# External dependencies
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
if (${spleeter_regenerate_models})
  include(add_spleeter)
else()
  include(add_spleeter_models)
endif()
include(add_eigen)
include(add_tensorflow)
include(add_rtff)
# --
if (WIN32)
  include(runtime)
  set_runtime(DYNAMIC)
  add_definitions(-DNOMINMAX)
endif()

include_directories(src)
add_subdirectory(src)

if (${spleeter_enable_tests})
  set(test_audio_path ${CMAKE_CURRENT_BINARY_DIR}/test.wav)
  if (NOT EXISTS ${test_audio_path})
    file(DOWNLOAD "https://archive.org/download/test_wav/Untitled3.wav" "${test_audio_path}")
  endif()
  include(add_googletest)
  include(add_wave)
  add_subdirectory(test)
endif()
