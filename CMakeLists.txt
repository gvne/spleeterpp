cmake_minimum_required(VERSION 3.14)

project(spleeter++)

set(CMAKE_CXX_STANDARD 14)

option(spleeter_enable_tests "Enable unit tests" ON)
option(spleeter_regenerate_models "Build the models from deezer/spleeter repository (requires conda)" OFF)
option(spleeter_enable_filter "Enable the filter interface (for online processing)" ON)
option(spleeter_enable_high_resolution "Process the spectrum up the 16KHz" OFF)

###############
# Filter specific options
if (${spleeter_enable_filter})
  # See https://github.com/deezer/spleeter/wiki/3.-Models#audio-parameters
  set(spleeter_input_frame_count 64 CACHE STRING "Time length of the input spectrogram segment expressed in short time Fourier transform frames")
  set(allowed_frame_count "64;128;256;512")
  if (NOT spleeter_input_frame_count IN_LIST allowed_frame_count)
    message(FATAL_ERROR "Can't use ${spleeter_input_frame_count} as spleeter_input_frame_count. Only 64/128/256/512 are supported")
  endif()
  message(STATUS "using spleeter_input_frame_count ${spleeter_input_frame_count}")
endif()
###############
# When regenerating the models, 16KHz is handled by the frequency bin count (F parameter in spleeter)
if (${spleeter_regenerate_models})
  set(spleeter_frequency_bin_count "1024")
  if (${spleeter_enable_high_resolution})
    set(spleeter_frequency_bin_count "1536")  # from here: https://github.com/deezer/spleeter/blob/master/spleeter/resources/4stems-16kHz.json
  endif()
  message(STATUS "using spleeter_frequency_bin_count ${spleeter_frequency_bin_count}")
endif()
###############

# External dependencies
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
if (${spleeter_regenerate_models})
  include(add_spleeter)
else()
  include(add_spleeter_models)
endif()
include(add_eigen)
include(add_tensorflow)
if (${spleeter_enable_filter})
  include(add_rtff)
endif()
# --
if (WIN32)
  include(runtime)
  set_runtime(DYNAMIC)
  add_definitions(-DNOMINMAX)
endif()

include_directories(src)

if (${spleeter_enable_tests})
  enable_testing()
  set(test_audio_path ${CMAKE_CURRENT_BINARY_DIR}/test.wav)
  if (NOT EXISTS ${test_audio_path})
    file(DOWNLOAD "https://archive.org/download/test_wav/Untitled3.wav" "${test_audio_path}")
  endif()
  include(add_googletest)
  include(googletest)
  include(add_wave)
  add_subdirectory(test)
endif()

add_subdirectory(src)
