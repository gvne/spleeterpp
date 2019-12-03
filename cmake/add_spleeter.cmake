include(FetchContent)

FetchContent_Declare(spleeter
  GIT_REPOSITORY https://github.com/deezer/spleeter.git
  GIT_TAG        v1.4.0
)

FetchContent_GetProperties(spleeter)
if(NOT spleeter_POPULATED)
  FetchContent_Populate(spleeter)
  # install instructions:
  # First install the conda environment:
  # $> conda install -c conda-forge spleeter
  # -- Make sure we have conda installed
  find_program(conda NAMES conda)
  if (NOT conda)
    message(FATAL_ERROR "conda couldn't be found. Please install it and try again")
  endif()
  message(STATUS "Found conda at ${conda}")
  message(STATUS "Installing Conda environment")
  execute_process(
    COMMAND ${conda} install -y -c conda-forge spleeter -p ${spleeter_BINARY_DIR}
    WORKING_DIRECTORY ${spleeter_SOURCE_DIR}
  )
  message(STATUS "Conda environment successfuly setup")
  set(spleeter_env_dir ${spleeter_BINARY_DIR})

  # TODO: download the pretrained models to avoid the necessity forcing the
  # internet connection...

endif()
