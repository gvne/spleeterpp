include(FetchContent)

FetchContent_Declare(rtff
  GIT_REPOSITORY https://github.com/gvne/rtff.git
  GIT_TAG        v0.1.1
)

FetchContent_GetProperties(rtff)
if(NOT rtff_POPULATED)
  FetchContent_Populate(rtff)
  set(rtff_enable_tests OFF CACHE BOOL "")
  # TODO: try to use the MKL backend. If not possible, fallback to the fftw on
  # UNIX and Eigen on windows
  if (UNIX)  # enable the fftw backend on linux and osx
    set(rtff_use_fftw ON CACHE BOOL "")
  endif()
  add_subdirectory(${rtff_SOURCE_DIR} ${rtff_BINARY_DIR})
endif()
