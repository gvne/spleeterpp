add_executable(test_olspleeter
  test_olspleeter.cc
)

target_link_libraries(test_olspleeter
  gtest
  gtest_main
  tensor
  spleeter
  ${tensorflow_cc}
  wave
  rtff
  mkl
)

target_include_directories(test_olspleeter
  PUBLIC
    ${tensorflow_include_dir}
)

target_compile_definitions(test_olspleeter
  PRIVATE
    SPLEETER_MODELS="${spleeter_models_dir}"
    TEST_FILE="${test_audio_path}"
    OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}"
    SPLEETER_INPUT_FRAME_COUNT=${spleeter_input_frame_count}
)
