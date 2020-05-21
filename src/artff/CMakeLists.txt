add_library(artff
  abstract_filter.cc
  abstract_filter.h
  buffer/circular_frame_buffer.h
  buffer/circular_frame_buffer.cc
  buffer/circular_future_buffer.h
  buffer/circular_future_buffer.cc
  semaphore.cc
  semaphore.h
)

target_link_libraries(artff
  rtff
)

install(TARGETS artff
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/abstract_filter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/semaphore.h
  DESTINATION include/artff/
)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/buffer/circular_frame_buffer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/buffer/circular_future_buffer.h
  DESTINATION include/artff/buffer
)
