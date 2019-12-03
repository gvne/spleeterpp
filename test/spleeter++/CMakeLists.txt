add_executable(spleeter_test
  test_basic.cc
)

target_link_libraries(spleeter_test
  gtest
  gtest_main
  pybind11::embed
)
target_compile_definitions(spleeter_test
  PRIVATE
    SPLEETER_ENV="${spleeter_env_dir}"
    TEST_FILE_PATH="${spleeter_env_dir}/../spleeter-src/audio_example.mp3"
)
