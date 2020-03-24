add_executable(test_spleeter_filter
  test_spleeter.cc
)

target_link_libraries(test_spleeter_filter
  gtest
  gtest_main
  tensorflow
  spleeter_filter
  wave
)

target_compile_definitions(test_spleeter_filter
  PRIVATE
    SPLEETER_MODELS="${spleeter_filter_models_dir}"
    TEST_FILE="${test_audio_path}"
    OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}"
    SPLEETER_INPUT_FRAME_COUNT=${spleeter_input_frame_count}
)
