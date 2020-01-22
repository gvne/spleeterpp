# Download the exported models and unzip it

set(spleeter_env_dir ${CMAKE_CURRENT_BINARY_DIR}/models)
set(zip_file models.zip)
set(zip_path ${spleeter_env_dir}/${zip_file})
set(expected_sha256 "7d78053099cd8b6d6afabee6bb17aa85f34346e78155b6f6691f4b6568a43ce2")
set(url "https://github.com/gvne/spleeterpp/releases/download/tf_cc-1.14.0/spleeter-models.zip")

file(MAKE_DIRECTORY ${spleeter_env_dir})
set(sha256 "")
if (EXISTS ${zip_path})
  file(SHA256 ${zip_path} sha256)
endif()

if (NOT "${sha256}" STREQUAL "${expected_sha256}")
  file(DOWNLOAD ${url} ${zip_path} SHOW_PROGRESS)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar -xf ${zip_file}
    WORKING_DIRECTORY ${spleeter_env_dir}
  )
endif()
set(spleeter_models_dir ${spleeter_env_dir}/exported)
