#include <grpcpp/grpcpp.h>
#include "file_processor.grpc.pb.h"
#include "file_processor_service_impl.h"

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  FileProcessorServiceImpl service;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Servidor gRPC ouvindo em " << server_address << std::endl;
  server->Wait();
}

int main() {
  RunServer();
  return 0;
}
