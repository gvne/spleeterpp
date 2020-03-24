add_library(spleeter_filter
  model.h
  model.cc
  filter.cc
  filter.h
  tensor.h
  tensor.cc
)

target_link_libraries(spleeter_filter
  spleeter_common
  tensorflow
  Eigen3::Eigen
  artff
  mkl  # TODO: don't force the use of mkl. Enable eigen fft ?
)

target_compile_definitions(spleeter_filter
  PRIVATE
    SPLEETER_INPUT_FRAME_COUNT=${spleeter_input_frame_count}
)

install(TARGETS spleeter_filter
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
