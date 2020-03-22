add_executable(test_spleeter
  test_spleeter.cc
)

target_link_libraries(test_spleeter
  gtest
  gtest_main
  tensorflow
  spleeter
  wave
)

target_compile_definitions(test_spleeter
  PRIVATE
    SPLEETER_MODELS="${spleeter_models_dir}"
    TEST_FILE="${test_audio_path}"
    OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}"
    SPLEETER_INPUT_FRAME_COUNT=${spleeter_input_frame_count}
)
