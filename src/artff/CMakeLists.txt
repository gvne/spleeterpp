add_library(artff
  abstract_filter.cc
  abstract_filter.h
  buffer/circular_frame_buffer.h
  buffer/circular_frame_buffer.cc
  buffer/circular_future_buffer.h
  buffer/circular_future_buffer.cc
)

target_link_libraries(artff
  rtff
)
