add_library(spleeter
  spleeter.h
  spleeter.cc
  registry.h
  registry.cc
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

# Add a simple standalone executable that uses the library
add_executable(spleeter_exe
  main.x.cc
)
target_link_libraries(spleeter_exe PUBLIC
  spleeter
  wave
)
target_compile_definitions(spleeter_exe PUBLIC
  SPLEETER_MODELS="${spleeter_models_dir}"
)
install(TARGETS spleeter_exe
  RUNTIME DESTINATION bin
)
