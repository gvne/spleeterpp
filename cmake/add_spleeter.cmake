include(FetchContent)

# reference to 16KHz commit from romi1502
# see https://github.com/deezer/spleeter/commit/fc1e3d9a2f620c481f9d876ee142be8787abc3d9
set(spleeter_tag fc1e3d9a2f620c481f9d876ee142be8787abc3d9)
set(spleeter_model_tag v1.4.0)
set(spleeter_conda_version 1.4.9)  # 1.5.0 changes options

FetchContent_Declare(spleeter
  GIT_REPOSITORY https://github.com/deezer/spleeter.git
  GIT_TAG        ${spleeter_tag}
  PATCH_COMMAND  git apply ${CMAKE_CURRENT_LIST_DIR}/patches/spleeter.patch
)

FetchContent_GetProperties(spleeter)
if(NOT spleeter_POPULATED)
  FetchContent_Populate(spleeter)
  # Install the conda environment:
  # $> conda install -c conda-forge spleeter
  # -- Make sure we have conda installed
  find_program(conda NAMES conda)
  if (NOT conda)
    message(FATAL_ERROR "conda couldn't be found. Please install it and try again")
  endif()
  message(STATUS "Found conda at ${conda}")
  message(STATUS "Installing Conda environment")
  # -- and install the environment
  execute_process(
    COMMAND ${conda} install -y -c conda-forge spleeter=${spleeter_conda_version} -p ${spleeter_BINARY_DIR}
    WORKING_DIRECTORY ${spleeter_SOURCE_DIR}
  )
  message(STATUS "Conda environment successfuly setup")
  set(spleeter_env_dir ${spleeter_BINARY_DIR})

  # Make sure we use the python executable of the spleeter environment
  set(PYTHON_EXECUTABLE ${spleeter_env_dir}/bin/python CACHE STRING "")

  # Download the pretrained models to avoid the internet at runtime
  set(pretrained_models "2stems" "4stems" "5stems")
  set(2stems_sha256 "f3a90b39dd2874269e8b05a48a86745df897b848c61f3958efc80a39152bd692")
  set(4stems_sha256 "3adb4a50ad4eb18c7c4d65fcf4cf2367a07d48408a5eb7d03cd20067429dfaa8")
  set(5stems_sha256 "25a1e87eb5f75cc72a4d2d5467a0a50ac75f05611f877c278793742513cc7218")
  set(pretrained_models_url "https://github.com/deezer/spleeter/releases/download/${spleeter_model_tag}/")
  set(pretrained_models_path "${spleeter_BINARY_DIR}/pretrained_models")
  file(MAKE_DIRECTORY ${pretrained_models_path})

  foreach(pretrained_model ${pretrained_models})
    set(pretrained_model_path "${pretrained_models_path}/${pretrained_model}")
    set(zip_file "${pretrained_model_path}/${pretrained_model}.tar.gz")
    set(url "${pretrained_models_url}${pretrained_model}.tar.gz")

    # Only download and unzip if the zip file wasn't downloaded earlier
    file(MAKE_DIRECTORY ${pretrained_model_path})
    set(sha256 "")
    if (EXISTS ${zip_file})
      file(SHA256 ${zip_file} sha256)
    endif()

    if (NOT "${${pretrained_model}_sha256}" STREQUAL "${sha256}")
      message(STATUS "Downloading ${pretrained_model} model at ${url} to ${zip_file}")
      file(DOWNLOAD ${url} ${zip_file} SHOW_PROGRESS)


      message(STATUS "Extracting ${zip_file} to ${pretrained_model_path}")
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar -xf ${zip_file}
        WORKING_DIRECTORY ${pretrained_model_path}
      )
      message(STATUS "${CMAKE_COMMAND} -E tar -xf ${zip_file}")
    else()
      message(STATUS "Pre-trained ${pretrained_model} already available.")
    endif()


  endforeach()

  # convert the downloaded models for tensorflow_cc
  set(spleeter_models_dir ${spleeter_env_dir}/exported)
  set(spleeter_filter_models_dir ${spleeter_env_dir}/filter_exported)
  if (NOT EXISTS ${spleeter_models_dir})
    message(STATUS "Exporing pre-trained models")

    execute_process(
      COMMAND
        ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/export_spleeter_models.py
          ${pretrained_models_path}
          ${spleeter_models_dir}
          ${spleeter_frequency_bin_count}
      WORKING_DIRECTORY ${spleeter_SOURCE_DIR}
    )
  endif()


  if (${spleeter_enable_filter} AND NOT EXISTS ${spleeter_filter_models_dir})
    message(STATUS "Exporing pre-trained models for filter interface")

    execute_process(
      COMMAND
        ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/export_spleeter_filter_models.py
          ${pretrained_models_path}
          ${spleeter_filter_models_dir}
          ${spleeter_input_frame_count}
          ${spleeter_frequency_bin_count}
      WORKING_DIRECTORY ${spleeter_SOURCE_DIR}
    )
  endif()

endif()
