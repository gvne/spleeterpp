set(spleeter_models_dir ${spleeter_env_dir}/exported)
if (NOT EXISTS ${spleeter_models_dir})  # TODO: if the command fails the folder may be corruped
  message(STATUS "Exporting models")
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/export_spleeter_models.py ${spleeter_env_dir}/pretrained_models ${spleeter_models_dir}
  )
endif()
