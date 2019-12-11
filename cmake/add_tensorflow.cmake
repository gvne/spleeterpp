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
set(tensorflow_include_dir ${tensorflow_include_dir} ${TENSORFLOW_CC_INSTALL_DIR}/include)
