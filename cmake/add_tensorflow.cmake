message(STATUS "Using search path: ${TENSORFLOW_CC_INSTALL_DIR}")

set(tensorflow_dir /Users/gvne/Downloads/libtensorflow-cpu-darwin-x86_64-1.15.0)
# TODO: use cmake commands to find the headers ?
set(tensorflow_include_dir ${tensorflow_dir}/include)

find_library(tensorflow_lib
  NAMES tensorflow
  PATHS ${tensorflow_dir}/lib
)
find_library(tensorflow_framework_lib
  NAMES tensorflow_framework
  PATHS ${tensorflow_dir}/lib
)
if(NOT tensorflow_lib OR NOT tensorflow_framework_lib)
  message(FATAL_ERROR "Couldn't find tensorflow libraries in ${tensorflow_dir}")
endif()

add_library(tensorflow INTERFACE)
target_link_libraries(tensorflow
  INTERFACE
    ${tensorflow_lib}
    ${tensorflow_framework_lib}
)
target_include_directories(tensorflow
  INTERFACE
    ${tensorflow_include_dir}
)

# # Search the tensorflow_cc library
# find_library(tensorflow_cc
#   NAMES tensorflow_cc
#   PATHS ${TENSORFLOW_CC_INSTALL_DIR}/bin
# )
#
# # If not found download a prebuild
# if(NOT tensorflow_cc)
#   message(STATUS "tensorflow_cc library not found.")
#   message(STATUS "Downloading a pre-built version")
#
#   # Determine the os type
#   set(os_type "linux")
#   if (WIN32)
#     message(FATAL_ERROR "Pre-built are not available for windows")
#   elseif(UNIX AND APPLE)
#     set(os_type "osx")
#   endif()
#
#   # Download the sources and unzip it
#   set(tensorflow_install_dir ${CMAKE_CURRENT_BINARY_DIR}/tensorflow)
#   file(DOWNLOAD
#     https://github.com/gvne/spleeterpp/releases/download/tf_cc-1.14.0/tf_cc-1.14.0-${os_type}.zip
#     ${tensorflow_install_dir}/tensorflow_cc.zip
#     SHOW_PROGRESS
#   )
#   execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf tensorflow_cc.zip
#                   WORKING_DIRECTORY ${tensorflow_install_dir})
#
#   set(TENSORFLOW_CC_INSTALL_DIR ${tensorflow_install_dir}/install CACHE STRING "" FORCE)
#   execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
#                   ${TENSORFLOW_CC_INSTALL_DIR}/bin/libtensorflow_cc.so.1
#                   ${TENSORFLOW_CC_INSTALL_DIR}/bin/libtensorflow_cc.so)
#
#   find_library(tensorflow_cc
#     NAMES tensorflow_cc
#     PATHS ${TENSORFLOW_CC_INSTALL_DIR}/bin
#   )
# endif()
#
# # If the prebuild failed we don't have any other option
# if(NOT tensorflow_cc)
#   message(FATAL_ERROR "tensorflow_cc library not found.")
# endif()
# message(STATUS "Found tensorflow_cc: ${tensorflow_cc}")
#
# set(tensorflow_include_dir ${TENSORFLOW_CC_INSTALL_DIR}/include)
#
# # eigen has to be reachable from a specific path. Add a symbolic link
# execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
#                 ${eigen3_SOURCE_DIR}/Eigen
#                 ${tensorflow_include_dir}/third_party/eigen3/Eigen)
