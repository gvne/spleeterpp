add_library(spleeter_common
  type.h
  registry.h
  registry.cc
  tf_handle.h
  tf_handle.cc
  spleeter_common.h
  spleeter_common.cc
)

target_link_libraries(spleeter_common
  tensorflow
  Eigen3::Eigen
)

install(TARGETS spleeter_common
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/spleeter_common.h
  ${CMAKE_CURRENT_SOURCE_DIR}/type.h
  DESTINATION include/spleeter_common/
)
