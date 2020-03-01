add_library(spleeter
  spleeter.h
  spleeter.cc
  type.h
  registry.h
  registry.cc
  model.h
  model.cc
)

target_link_libraries(spleeter
  ${tensorflow_cc}
  Eigen3::Eigen
)

target_include_directories(spleeter
  PUBLIC
    ${tensorflow_include_dir}
    ${protobuf_include_dir}
    ${absl_include_dir}
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
install(FILES
  ${tensorflow_cc}
  DESTINATION lib
)

# Also install the pretrained models
install(DIRECTORY
  ${spleeter_models_dir}/
  DESTINATION models
)
