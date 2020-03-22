add_library(spleeter
  spleeter.h
  spleeter.cc
  type.h
  registry.h
  registry.cc
  model.h
  model.cc
  filter.cc
  filter.h
  tensor.h
  tensor.cc
  tf_handle.h
  tf_handle.cc
)

target_link_libraries(spleeter
  tensorflow
  Eigen3::Eigen
  artff
  mkl  # TODO: don't force the use of mkl. Enable eigen fft ?
)

target_compile_definitions(spleeter
  PRIVATE
    SPLEETER_INPUT_FRAME_COUNT=${spleeter_input_frame_count}
)

install(TARGETS spleeter
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/spleeter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/type.h
  DESTINATION include/spleeter
)

# Also install the pretrained models
install(DIRECTORY
  ${spleeter_models_dir}/
  DESTINATION models
)
