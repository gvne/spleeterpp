set(tensorflow_dir ${CMAKE_CURRENT_BINARY_DIR}/tensorflow)

# On OSX, we want to install the libtensorflow.1.dylib
set(tensorflow_lib_name tensorflow)
set(tensorflow_framework_name tensorflow_framework)

# Find the libraries again
find_library(tensorflow_lib
  NAMES ${tensorflow_lib_name}
  PATHS ${tensorflow_dir}/lib
)
if (UNIX)
  find_library(tensorflow_framework_lib
    NAMES ${tensorflow_framework_name}
    PATHS ${tensorflow_dir}/lib
  )
endif()

# If not found
if (NOT tensorflow_lib)
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

  # On unix, we need to remove the symlinks for simpler install process
  if (UNIX)
    if (APPLE)
      file(REMOVE ${tensorflow_dir}/lib/libtensorflow_framework.1.dylib)
      file(RENAME ${tensorflow_dir}/lib/libtensorflow_framework.1.15.0.dylib ${tensorflow_dir}/lib/libtensorflow_framework.1.dylib)
      file(REMOVE ${tensorflow_dir}/lib/libtensorflow.1.dylib)
      file(RENAME ${tensorflow_dir}/lib/libtensorflow.1.15.0.dylib ${tensorflow_dir}/lib/libtensorflow.1.dylib)
    else()
      file(REMOVE ${tensorflow_dir}/lib/libtensorflow_framework.so.1)
      file(RENAME ${tensorflow_dir}/lib/libtensorflow_framework.so.1.15.0 ${tensorflow_dir}/lib/libtensorflow_framework.so.1)
      file(REMOVE ${tensorflow_dir}/lib/libtensorflow.so.1)
      file(RENAME ${tensorflow_dir}/lib/libtensorflow.so.1.15.0 ${tensorflow_dir}/lib/libtensorflow.so.1)
    endif()
  endif()
endif()

# Find the libraries again
find_library(tensorflow_lib
  NAMES ${tensorflow_lib_name}
  PATHS ${tensorflow_dir}/lib
)
if (NOT tensorflow_lib)
  message(FATAL_ERROR "Tensorflow could not be included")
endif()

add_library(tensorflow INTERFACE)
if (WIN32)
  # On windows we don't have and don't need the tensorflow_framework library
  set(tensorflow_libs ${tensorflow_lib})
else()
  find_library(tensorflow_framework_lib
    NAMES ${tensorflow_framework_name}
    PATHS ${tensorflow_dir}/lib
    REQUIRED
  )
  if (NOT tensorflow_framework_lib)
    message(FATAL_ERROR "Tensorflow framework could not be included")
  endif()

  set(tensorflow_libs ${tensorflow_lib} ${tensorflow_framework_lib})
endif()
target_link_libraries(tensorflow
  INTERFACE ${tensorflow_libs}
)
target_include_directories(tensorflow
  INTERFACE
    ${tensorflow_dir}/include
)

# install tensorflow
if(IS_SYMLINK ${tensorflow_lib})
  get_filename_component(filename ${tensorflow_lib} NAME)
  get_filename_component(dir ${tensorflow_lib} DIRECTORY)
  file(READ_SYMLINK "${tensorflow_lib}" raw_symlink)
  INSTALL(FILES ${dir}/${raw_symlink} DESTINATION lib)
else()
  INSTALL(FILES ${tensorflow_lib} DESTINATION lib)
endif()

# Also install the dll on windows
if (WIN32)
  install(FILES ${tensorflow_dir}/lib/tensorflow.dll DESTINATION lib)
endif()

# And tensorflow framework
if (NOT WIN32)
  if(IS_SYMLINK ${tensorflow_framework_lib})
    get_filename_component(filename ${tensorflow_framework_lib} NAME)
    get_filename_component(dir ${tensorflow_framework_lib} DIRECTORY)
    file(READ_SYMLINK "${tensorflow_framework_lib}" raw_symlink)
    INSTALL(FILES ${dir}/${raw_symlink} DESTINATION lib)
  else()
    INSTALL(FILES ${tensorflow_framework_lib} DESTINATION lib)
  endif()
endif()

# Also install a symlinks
if (UNIX)
  if (APPLE)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/libtensorflow.1.dylib ${CMAKE_INSTALL_PREFIX}/lib/libtensorflow.dylib)")
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/libtensorflow_framework.1.dylib ${CMAKE_INSTALL_PREFIX}/lib/libtensorflow_framework.dylib)")
  else()
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/libtensorflow.so.1 ${CMAKE_INSTALL_PREFIX}/lib/libtensorflow.so)")
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/libtensorflow_framework.so.1 ${CMAKE_INSTALL_PREFIX}/lib/libtensorflow_framework.so)")
  endif()
endif()
