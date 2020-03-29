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
  ${CMAKE_CURRENT_SOURCE_DIR}/filter.h
  DESTINATION include/spleeter_filter
)

# Also install the pretrained models
install(DIRECTORY
  ${spleeter_filter_models_dir}/
  DESTINATION models/filter
)
