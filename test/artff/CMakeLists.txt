add_executable(test_artff
  test_artff.cc
)

target_link_libraries(test_artff
  gtest
  gtest_main
  artff
  wave
)

target_compile_definitions(test_artff
  PRIVATE
    TEST_FILE="${test_audio_path}"
    OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)
