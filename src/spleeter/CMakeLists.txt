add_library(spleeter
  spleeter.h
  spleeter.hxx
  spleeter.cc
  registry.h
  registry.cc
)

target_link_libraries(spleeter
  ${tensorflow_cc}
)

target_include_directories(spleeter
  PUBLIC ${tensorflow_include_dir}
)
