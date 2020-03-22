add_library(spleeter
  spleeter.h
  spleeter.cc
  registry.h
  registry.cc
  tf_handle.h
  tf_handle.cc
)

target_link_libraries(spleeter
  tensorflow
  Eigen3::Eigen
)

install(TARGETS spleeter
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/spleeter.h
  DESTINATION include/spleeter
)

# Also install the pretrained models
install(DIRECTORY
  ${spleeter_models_dir}/
  DESTINATION models
)
