set(tensorflow_dir ${CMAKE_CURRENT_BINARY_DIR}/tensorflow)

# Find the libraries
find_library(tensorflow_lib
  NAMES tensorflow
  PATHS ${tensorflow_dir}/lib
)
find_library(tensorflow_framework_lib
  NAMES tensorflow_framework
  PATHS ${tensorflow_dir}/lib
)

# If not found
if (NOT tensorflow_lib OR NOT tensorflow_framework_lib)
  message(STATUS "Downloading tensorflow C API pre-built")

  # Download
  if (UNIX AND NOT APPLE)  # Linux
    set(tf_url "https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-cpu-linux-x86_64-1.15.0.tar.gz")
  elseif (UNIX AND APPLE)  # OSX
    set(tf_url "https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-cpu-darwin-x86_64-1.15.0.tar.gz")
  else()                   # Windows
    set(tf_url "https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-cpu-windows-x86_64-1.15.0.zip")
  endif()

  file(DOWNLOAD
    ${tf_url}
    ${tensorflow_dir}/tensorflow_cc.zip
    SHOW_PROGRESS
  )
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf tensorflow_cc.zip
                  WORKING_DIRECTORY ${tensorflow_dir})

  # Find the libraries again
  find_library(tensorflow_lib
    NAMES tensorflow
    PATHS ${tensorflow_dir}/lib
  )
  find_library(tensorflow_framework_lib
    NAMES tensorflow_framework
    PATHS ${tensorflow_dir}/lib
  )
  if (NOT tensorflow_lib OR NOT tensorflow_framework_lib)
    message(FATAL_ERROR "Tensorflow could not be included")
  endif()
endif()

# -- Create lib target
add_library(tensorflow INTERFACE)
target_link_libraries(tensorflow
  INTERFACE
    ${tensorflow_lib}
    ${tensorflow_framework_lib}
)
target_include_directories(tensorflow
  INTERFACE
    ${tensorflow_dir}/include
)
INSTALL(FILES ${tensorflow_lib} ${tensorflow_framework_lib}
  DESTINATION lib
)
