include(FetchContent)

FetchContent_Declare(Eigen3
  GIT_REPOSITORY https://github.com/eigenteam/eigen-git-mirror.git
  GIT_TAG 3.3.7
)

FetchContent_GetProperties(Eigen3)
if(NOT eigen3_POPULATED)
  FetchContent_Populate(Eigen3)
  add_library(eigen INTERFACE)
  target_include_directories(eigen INTERFACE ${eigen3_SOURCE_DIR})
  add_library(Eigen3::Eigen ALIAS eigen)
endif()
