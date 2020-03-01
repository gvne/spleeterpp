add_library(tensor
  copy.cc
  copy.h
)

target_link_libraries(tensor
  ${tensorflow_cc}
  Eigen3::Eigen
)

target_include_directories(tensor PUBLIC
  ${tensorflow_include_dir}
  ${protobuf_include_dir}
  ${absl_include_dir}
)
