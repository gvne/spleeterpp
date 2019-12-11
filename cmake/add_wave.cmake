include(FetchContent)

FetchContent_Declare(
  wave
  GIT_REPOSITORY https://github.com/gvne/wave.git
  GIT_TAG        develop
)

FetchContent_GetProperties(wave)
if(NOT wave_POPULATED)
  FetchContent_Populate(wave)
  set(wave_enable_tests OFF CACHE BOOL "")
  add_subdirectory(${wave_SOURCE_DIR} ${wave_BINARY_DIR})
endif()
