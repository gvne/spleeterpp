if (NOT EXISTS ${TENSORFLOW_CC_INSTALL_DIR})
  message(STATUS "Downloading pre-built version of tensorflow_cc library")
  if (WIN32)
    message(FATAL_ERROR "Pre-built are not available for windows")
  elseif(UNIX AND APPLE)
    set(url https://github.com/gvne/spleeterpp/releases/download/tf_cc-1.14.0/tf_cc-1.14.0-osx.zip)
  else()
    set(url https://github.com/gvne/spleeterpp/releases/download/tf_cc-1.14.0/tf_cc-1.14.0-linux.zip)
  endif()
  set(zipped_lib tensorflow_cc.zip)
  # TODO: use md5 to avoid redownloading instead of this check
  if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${zipped_lib})
    file(DOWNLOAD ${url} ${CMAKE_CURRENT_BINARY_DIR}/${zipped_lib} SHOW_PROGRESS)
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf ${zipped_lib}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set(TENSORFLOW_CC_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/install)
  if (UNIX AND APPLE)  # TODO: should be included in the zip
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
                    ${TENSORFLOW_CC_INSTALL_DIR}/bin/libtensorflow_cc.so
                    ${TENSORFLOW_CC_INSTALL_DIR}/bin/libtensorflow_cc.so.1)
  endif()
endif()

find_library(tensorflow_cc
  NAMES tensorflow_cc
  PATHS ${TENSORFLOW_CC_INSTALL_DIR}/bin
  REQUIRED
)
if(NOT tensorflow_cc)
  message(FATAL_ERROR "tensorflow_cc library not found. See README.md to build it from sources")
endif()
message(STATUS "Found tensorflow_cc: ${tensorflow_cc}")

# find tensorflow path
execute_process(COMMAND
  ${PYTHON_EXECUTABLE} "-c" "import tensorflow as tf; import os; print(os.path.dirname(tf.__file__))"
  OUTPUT_VARIABLE tensorflow_lib_dir
  ERROR_QUIET
)
string(STRIP ${tensorflow_lib_dir} tensorflow_lib_dir)

# Make sure we installed all the dependencies
execute_process(COMMAND
  ${PYTHON_EXECUTABLE} "${tensorflow_lib_dir}/tools/pip_package/setup.py" "install"
  OUTPUT_QUIET
)

# find include dir
execute_process(COMMAND
  ${PYTHON_EXECUTABLE} "-c" "import tensorflow as tf; print(tf.sysconfig.get_include())"
  OUTPUT_VARIABLE tensorflow_include_dir
  ERROR_QUIET
)
string(STRIP ${tensorflow_include_dir} tensorflow_include_dir)
set(tensorflow_include_dir ${tensorflow_include_dir} ${TENSORFLOW_CC_INSTALL_DIR}/include)
