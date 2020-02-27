add_executable(test_spleeter
  # test_loader.cc  # Just a sandbox
  test_spleeter.cc
)

target_link_libraries(test_spleeter
  gtest
  gtest_main
  spleeter
  ${tensorflow_cc}
  wave
  rtff
  mkl
)

target_include_directories(test_spleeter PUBLIC ${tensorflow_include_dir})

target_compile_definitions(test_spleeter
  PRIVATE
    SPLEETER_MODELS="${spleeter_models_dir}"
    TEST_FILE="${test_audio_path}"
    OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}"
    SPLEETER_INPUT_FRAME_COUNT=${spleeter_input_frame_count}
)
