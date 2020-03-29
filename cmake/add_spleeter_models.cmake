# Download the exported models and unzip it
set(spleeter_env_dir ${CMAKE_CURRENT_BINARY_DIR}/models)
file(MAKE_DIRECTORY ${spleeter_env_dir})

# ----------------------------------
# classic models
# ----------------------------------
set(spleeter_models_dir ${spleeter_env_dir}/offline)
set(zip_file models.zip)
set(zip_path ${spleeter_models_dir}/${zip_file})
set(expected_sha256 "edfdbdb9b830698fc859afbb9f1e5d8b2a7da1ec33ed1ccc288e9a57ef0c4ca2")
set(url "https://github.com/gvne/spleeterpp/releases/download/models-1.0/models.zip")

file(MAKE_DIRECTORY ${spleeter_models_dir})
set(sha256 "")
if (EXISTS ${zip_path})
  file(SHA256 ${zip_path} sha256)
endif()

if (NOT "${sha256}" STREQUAL "${expected_sha256}")
  file(DOWNLOAD ${url} ${zip_path} SHOW_PROGRESS)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar -xf ${zip_file}
    WORKING_DIRECTORY ${spleeter_models_dir}
  )
endif()

# ----------------------------------
# real-time models
# ----------------------------------
set(spleeter_filter_models_dir ${spleeter_env_dir}/online)
set(zip_file models.zip)
set(zip_path ${spleeter_filter_models_dir}/${zip_file})
set(expected_sha256 "3b1f7bc0c495b1b4ecb2c6e43d7bbb561a1fe499bfa16b15e3795687aa287257")
set(url "https://github.com/gvne/spleeterpp/releases/download/olmodels-v1.0/models.zip")

file(MAKE_DIRECTORY ${spleeter_filter_models_dir})
set(sha256 "")
if (EXISTS ${zip_path})
  file(SHA256 ${zip_path} sha256)
endif()

if (NOT "${sha256}" STREQUAL "${expected_sha256}")
  file(DOWNLOAD ${url} ${zip_path} SHOW_PROGRESS)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar -xf ${zip_file}
    WORKING_DIRECTORY ${spleeter_filter_models_dir}
  )
endif()
