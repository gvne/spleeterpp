add_executable(example MACOSX_BUNDLE
  main.cc
)
target_link_libraries(example spleeter++)
target_compile_definitions(example
  PRIVATE
    TEST_FILE_PATH="${spleeter_env_dir}/../spleeter-src/audio_example.mp3"
)
# TODO: only if the folder doesn't exist already ? ...
add_custom_command(TARGET example
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${spleeter_env_dir} $<TARGET_FILE_DIR:example>/../Resources
)
