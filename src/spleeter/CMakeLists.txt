add_library(spleeter
  spleeter.h
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

install(TARGETS spleeter
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/spleeter.h
  DESTINATION include/spleeter
)
install(FILES
  ${tensorflow_cc}
  DESTINATION lib
)
if (APPLE)
  install(FILES
    ${tensorflow_cc}.1
    DESTINATION lib
  )
endif()
