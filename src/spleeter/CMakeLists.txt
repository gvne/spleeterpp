add_library(spleeter
  model.cc
  model.h
  spleeter.cc
  spleeter.h
)

target_link_libraries(spleeter
  spleeter_common
  Eigen3::Eigen
)
