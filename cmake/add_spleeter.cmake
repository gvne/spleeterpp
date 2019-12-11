include(FetchContent)
set(spleeter_version v1.4.0)

FetchContent_Declare(spleeter
  GIT_REPOSITORY https://github.com/deezer/spleeter.git
  GIT_TAG        ${spleeter_version}
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
    COMMAND ${conda} install -y -c conda-forge spleeter -p ${spleeter_BINARY_DIR}
    WORKING_DIRECTORY ${spleeter_SOURCE_DIR}
  )
  message(STATUS "Conda environment successfuly setup")
  set(spleeter_env_dir ${spleeter_BINARY_DIR})

  # Make sure we use the python executable of the spleeter environment
  set(PYTHON_EXECUTABLE ${spleeter_env_dir}/bin/python CACHE STRING "")

  # Download the pretrained models to avoid the internet at runtime
  set(pretrained_models "2stems" "4stems" "5stems")
  set(pretrained_models_url "https://github.com/deezer/spleeter/releases/download/${spleeter_version}/")
  set(pretrained_models_path "${spleeter_BINARY_DIR}/pretrained_models")
  file(MAKE_DIRECTORY ${pretrained_models_path})

  foreach(pretrained_model ${pretrained_models})
    set(pretrained_model_path "${pretrained_models_path}/${pretrained_model}")
    set(zip_file "${pretrained_model_path}/${pretrained_model}.tar.gz")
    set(url "${pretrained_models_url}${pretrained_model}.tar.gz")

    message(STATUS "Downloading ${pretrained_model} model at ${url} to ${zip_file}")

    # Only download and unzip if the zip file wasn't downloaded earlier
    file(MAKE_DIRECTORY ${pretrained_model_path})
    if(NOT EXISTS ${zip_file})
      file(DOWNLOAD ${url} ${zip_file} SHOW_PROGRESS)
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar -xf ${zip_file}
        WORKING_DIRECTORY ${pretrained_model_path}
      )
    else()
      message(STATUS "Already downloaded. Skipping.")
    endif()
  endforeach()

endif()
