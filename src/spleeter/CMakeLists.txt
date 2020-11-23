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

install(TARGETS spleeter
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/spleeter.h
  DESTINATION include/spleeter/
)

# Also install the pretrained models
install(DIRECTORY
  ${spleeter_models_dir}/
  DESTINATION models/default
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
