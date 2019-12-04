add_executable(spleeter_test
  setup.cc
  test_separate.cc
)

target_link_libraries(spleeter_test
  gtest
  gtest_main
  spleeter++
)

target_compile_definitions(spleeter_test
  PRIVATE
    SPLEETER_ENV="${spleeter_env_dir}"
    TEST_FILE_PATH="${spleeter_env_dir}/../spleeter-src/audio_example.mp3"
)
