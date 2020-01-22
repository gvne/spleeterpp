include(FetchContent)

FetchContent_Declare(protobuf
  GIT_REPOSITORY https://github.com/protocolbuffers/protobuf
  GIT_TAG v3.7.0
)
FetchContent_GetProperties(protobuf)
if(NOT protobuf_POPULATED)
  FetchContent_Populate(protobuf)
  set(protobuf_include_dir ${protobuf_SOURCE_DIR}/src)
endif()
