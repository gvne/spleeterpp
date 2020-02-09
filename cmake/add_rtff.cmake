include(FetchContent)

FetchContent_Declare(rtff
  GIT_REPOSITORY https://github.com/gvne/rtff.git
  GIT_TAG        acca2d26feb10453e28dc7b4cc369f4f98b4afa6
)

FetchContent_GetProperties(rtff)
if(NOT rtff_POPULATED)
  FetchContent_Populate(rtff)
  set(rtff_enable_tests OFF CACHE BOOL "")
  add_subdirectory(${rtff_SOURCE_DIR} ${rtff_BINARY_DIR})
endif()
