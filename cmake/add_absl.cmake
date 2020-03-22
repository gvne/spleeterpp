include(FetchContent)

FetchContent_Declare(absl
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG master
)
FetchContent_GetProperties(absl)
if(NOT absl_POPULATED)
  FetchContent_Populate(absl)
  set(absl_include_dir ${absl_SOURCE_DIR})
endif()
