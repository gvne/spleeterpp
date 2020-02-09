include(FetchContent)

FetchContent_Declare(rtff
  GIT_REPOSITORY https://github.com/gvne/rtff.git
  GIT_TAG        71e5789a27dad63a16593538e3a971089aa407da
)

FetchContent_GetProperties(rtff)
if(NOT rtff_POPULATED)
  FetchContent_Populate(rtff)
  set(rtff_enable_tests OFF CACHE BOOL "")
  add_subdirectory(${rtff_SOURCE_DIR} ${rtff_BINARY_DIR})
endif()
