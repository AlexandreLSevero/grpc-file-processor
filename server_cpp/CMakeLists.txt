cmake_minimum_required(VERSION 3.10)
project(gRPCServer)

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/file_processor.pb.cc")
set(PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/file_processor.pb.h")
set(GRPC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/file_processor.grpc.pb.cc")
set(GRPC_HDRS "${CMAKE_CURRENT_BINARY_DIR}/file_processor.grpc.pb.h")

add_custom_command(
  OUTPUT "${PROTO_SRCS}" "${PROTO_HDRS}" "${GRPC_SRCS}" "${GRPC_HDRS}"
  COMMAND protobuf::protoc
  ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}" --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
       -I "${CMAKE_SOURCE_DIR}/../proto"
       --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
       file_processor.proto
  DEPENDS "${CMAKE_SOURCE_DIR}/../proto/file_processor.proto"
)

add_executable(server server.cc file_processor_service_impl.cc ${PROTO_SRCS} ${GRPC_SRCS})
target_link_libraries(server gRPC::grpc++ protobuf::libprotobuf)
target_include_directories(server PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
