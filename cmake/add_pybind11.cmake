include(FetchContent)

FetchContent_Declare(pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.4.3
)

FetchContent_GetProperties(pybind11)
if(NOT pybind11_POPULATED)
  FetchContent_Populate(pybind11)
  message(STATUS "Using python path: ${PYTHON_EXECUTABLE}")
  add_subdirectory(${pybind11_SOURCE_DIR} ${pybind11_BINARY_DIR})
endif()
