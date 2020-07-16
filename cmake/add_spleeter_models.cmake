# Download the exported models and unzip it
set(spleeter_env_dir ${CMAKE_CURRENT_BINARY_DIR}/models)
file(MAKE_DIRECTORY ${spleeter_env_dir})

# ----------------------------------
# classic models
# ----------------------------------
set(spleeter_models_dir ${spleeter_env_dir}/offline)
set(zip_file offline_models.zip)
set(zip_path ${spleeter_env_dir}/${zip_file})
set(expected_sha256 "edfdbdb9b830698fc859afbb9f1e5d8b2a7da1ec33ed1ccc288e9a57ef0c4ca2")
set(url "https://github.com/gvne/spleeterpp/releases/download/models-1.0/models.zip")
if (${spleeter_enable_high_resolution})
  set(url "https://github.com/gvne/spleeterpp/releases/download/models-1.0/models-16KHz.zip")
endif()

file(MAKE_DIRECTORY ${spleeter_models_dir})
set(sha256 "")
if (EXISTS ${zip_path})
  file(SHA256 ${zip_path} sha256)
endif()

if (NOT "${sha256}" STREQUAL "${expected_sha256}")
  message(STATUS "Downloading models from ${url}")
  file(DOWNLOAD ${url} ${zip_path} SHOW_PROGRESS)
  file(COPY ${zip_path} DESTINATION ${spleeter_models_dir})
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar -xf ${zip_file}
    WORKING_DIRECTORY ${spleeter_models_dir}
  )
  file(REMOVE ${spleeter_models_dir}/${zip_file})  # Cleanup
endif()

# ----------------------------------
# real-time models
# ----------------------------------
if (${spleeter_enable_filter})
  set(spleeter_filter_models_dir ${spleeter_env_dir}/online)
  set(zip_file online_models.zip)
  set(zip_path ${spleeter_env_dir}/${zip_file})
  set(expected_sha256 "3b1f7bc0c495b1b4ecb2c6e43d7bbb561a1fe499bfa16b15e3795687aa287257")
  set(url "https://github.com/gvne/spleeterpp/releases/download/olmodels-v1.0/models.zip")
  if (${spleeter_enable_high_resolution})
    set(url "https://github.com/gvne/spleeterpp/releases/download/olmodels-v1.0/models-16KHz.zip")
  endif()

  file(MAKE_DIRECTORY ${spleeter_filter_models_dir})
  set(sha256 "")
  if (EXISTS ${zip_path})
    file(SHA256 ${zip_path} sha256)
  endif()

  if (NOT "${sha256}" STREQUAL "${expected_sha256}")
    message(STATUS "Downloading online models from ${url}")
    file(DOWNLOAD ${url} ${zip_path} SHOW_PROGRESS)
    file(COPY ${zip_path} DESTINATION ${spleeter_filter_models_dir})
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar -xf ${zip_file}
      WORKING_DIRECTORY ${spleeter_filter_models_dir}
    )
    file(REMOVE ${spleeter_filter_models_dir}/${zip_file})  # Cleanup
  endif()
endif()
