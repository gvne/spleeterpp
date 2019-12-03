add_library(spleeter++
  spleeter++.h
  spleeter++.cc
)

target_link_libraries(spleeter++
  pybind11::embed
)
