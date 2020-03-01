add_library(olspleeter
  filter.cc
  filter.h
)

target_link_libraries(olspleeter
  ${tensorflow_cc}
  Eigen3::Eigen
  rtff
  mkl
)

target_include_directories(olspleeter
  PUBLIC
    ${tensorflow_include_dir}
    ${protobuf_include_dir}
    ${absl_include_dir}
)

target_compile_definitions(olspleeter
  PRIVATE
    SPLEETER_INPUT_FRAME_COUNT=${spleeter_input_frame_count}
)
