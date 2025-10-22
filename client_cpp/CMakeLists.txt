cmake_minimum_required(VERSION 3.10)
project(gRPCClient)

# Encontrar pacotes necessários
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Definir arquivos gerados pelo protoc
set(PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/file_processor.pb.cc")
set(PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/file_processor.pb.h")
set(GRPC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/file_processor.grpc.pb.cc")
set(GRPC_HDRS "${CMAKE_CURRENT_BINARY_DIR}/file_processor.grpc.pb.h")

# Comando para gerar código gRPC a partir do .proto
add_custom_command(
  OUTPUT "${PROTO_SRCS}" "${PROTO_HDRS}" "${GRPC_SRCS}" "${GRPC_HDRS}"
  COMMAND protobuf::protoc
  ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}" --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
       -I "${CMAKE_SOURCE_DIR}/../proto"
       --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
       file_processor.proto
  DEPENDS "${CMAKE_SOURCE_DIR}/../proto/file_processor.proto"
)

# Adicionar executável do cliente
add_executable(client client.cc ${PROTO_SRCS} ${GRPC_SRCS})

# Vincular bibliotecas
target_link_libraries(client gRPC::grpc++ protobuf::libprotobuf)

# Incluir diretórios
target_include_directories(client PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
