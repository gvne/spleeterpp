add_executable(test_load
  test_load.cc
)

target_link_libraries(test_load
  gtest
  gtest_main
  ${tensorflow_cc}
  wave
)

target_include_directories(test_load PUBLIC ${tensorflow_include_dir})

target_compile_definitions(test_load
  PRIVATE
    SPLEETER_MODELS="${spleeter_models_dir}"
    TEST_FILE="${test_audio_path}"
    OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)
