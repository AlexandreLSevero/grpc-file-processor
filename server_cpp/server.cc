#include <grpcpp/grpcpp.h>
#include "file_processor.grpc.pb.h"
#include "file_processor_service_impl.h"

// Função para iniciar o servidor gRPC
void RunServer() {
  // endereço e porta onde o servidor vai ouvir
  std::string server_address("0.0.0.0:50051");

  // Instancia a implementação do serviço
  FileProcessorServiceImpl service;

  // Configura o servidor com gRPC
  grpc::ServerBuilder builder;

  // Adiciona a porta de escuta com credenciais inseguras
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

  // Registra o serviço no servidor
  builder.RegisterService(&service);

  // Constrói e inicia o servidor
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Servidor gRPC ouvindo em " << server_address << std::endl;

  // Mantém o servidor rodando até ser interrompido
  server->Wait();
}

int main() {
  RunServer();
  return 0;
}
