#include <iostream>
#include <fstream>
#include <string>
#include <grpcpp/grpcpp.h>
#include "file_processor.grpc.pb.h"

// Usa namespaces para simplificar
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using namespace file_processor;

// Classe para o cliente gRPC
class FileProcessorClient {
public:
  // Construtor que inicializa o stub gRPC
  FileProcessorClient(std::shared_ptr<Channel> channel) : stub_(FileProcessorService::NewStub(channel)) {}

  // Método para chamar o serviço CompressPDF
  void CompressPDF(const std::string& input_path) {
    ClientContext context;
    // Cria um stream bidirecional para CompressPDF
    auto stream = stub_->CompressPDF(&context);
    // Extrai o nome do arquivo do caminho
    std::string file_name = input_path.substr(input_path.find_last_of("/") + 1);

    // Envia metadados com o nome do arquivo
    CompressRequestChunk req_meta;
    req_meta.mutable_metadata()->set_file_name(file_name);
    stream->Write(req_meta);

    // Envia o arquivo em chunks
    std::ifstream input_stream(input_path, std::ios::binary);
    char buffer[1024];
    while (input_stream.read(buffer, sizeof(buffer)) || input_stream.gcount() > 0) {
      CompressRequestChunk req_chunk;
      req_chunk.mutable_chunk()->set_content(buffer, input_stream.gcount());
      stream->Write(req_chunk);
    }
    stream->WritesDone(); // Fim do envio

    // Recebe a resposta do servidor
    ResponseChunk resp;
    std::string output_path = "compressed_" + file_name;
    std::ofstream output_stream(output_path, std::ios::binary);
    bool success = false;
    while (stream->Read(&resp)) {
      if (resp.has_metadata()) {
        // Processa metadados (status e mensagem)
        success = resp.metadata().success();
        std::cout << resp.metadata().status_message() << std::endl;
        if (!success) break;
      } else if (resp.has_chunk()) {
        // Salva o chunk no arquivo de saída
        output_stream.write(resp.chunk().content().data(), resp.chunk().content().size());
      }
    }
    output_stream.close();
    if (success) std::cout << "Salvo em: " << output_path << std::endl;
  }

  // Método para chamar o serviço ConvertToTXT
  void ConvertToTXT(const std::string& input_path) {
    ClientContext context;
    auto stream = stub_->ConvertToTXT(&context);
    std::string file_name = input_path.substr(input_path.find_last_of("/") + 1);

    // Envia metadados
    ConvertToTXTRequestChunk req_meta;
    req_meta.mutable_metadata()->set_file_name(file_name);
    stream->Write(req_meta);

    // Envia o arquivo em chunks
    std::ifstream input_stream(input_path, std::ios::binary);
    char buffer[1024];
    while (input_stream.read(buffer, sizeof(buffer)) || input_stream.gcount() > 0) {
      ConvertToTXTRequestChunk req_chunk;
      req_chunk.mutable_chunk()->set_content(buffer, input_stream.gcount());
      stream->Write(req_chunk);
    }
    stream->WritesDone();

    // Recebe a resposta
    ResponseChunk resp;
    std::string output_path = file_name + ".txt";
    std::ofstream output_stream(output_path, std::ios::binary);
    bool success = false;
    while (stream->Read(&resp)) {
      if (resp.has_metadata()) {
        success = resp.metadata().success();
        std::cout << resp.metadata().status_message() << std::endl;
        if (!success) break;
      } else if (resp.has_chunk()) {
        output_stream.write(resp.chunk().content().data(), resp.chunk().content().size());
      }
    }
    output_stream.close();
    if (success) std::cout << "Salvo em: " << output_path << std::endl;
  }

  // Método para chamar o serviço ConvertImageFormat
  void ConvertImageFormat(const std::string& input_path, const std::string& output_format) {
    ClientContext context;
    auto stream = stub_->ConvertImageFormat(&context);
    std::string file_name = input_path.substr(input_path.find_last_of("/") + 1);

    // Envia metadados com nome e formato de saída
    ConvertImageFormatRequestChunk req_meta;
    req_meta.mutable_metadata()->set_file_name(file_name);
    req_meta.mutable_metadata()->set_output_format(output_format);
    stream->Write(req_meta);

    // Envia o arquivo em chunks
    std::ifstream input_stream(input_path, std::ios::binary);
    char buffer[1024];
    while (input_stream.read(buffer, sizeof(buffer)) || input_stream.gcount() > 0) {
      ConvertImageFormatRequestChunk req_chunk;
      req_chunk.mutable_chunk()->set_content(buffer, input_stream.gcount());
      stream->Write(req_chunk);
    }
    stream->WritesDone();

    // Recebe a resposta
    ResponseChunk resp;
    std::string output_path = "converted." + output_format;
    std::ofstream output_stream(output_path, std::ios::binary);
    bool success = false;
    while (stream->Read(&resp)) {
      if (resp.has_metadata()) {
        success = resp.metadata().success();
        std::cout << resp.metadata().status_message() << std::endl;
        if (!success) break;
      } else if (resp.has_chunk()) {
        output_stream.write(resp.chunk().content().data(), resp.chunk().content().size());
      }
    }
    output_stream.close();
    if (success) std::cout << "Salvo em: " << output_path << std::endl;
  }

  // Método para chamar o serviço ResizeImage
  void ResizeImage(const std::string& input_path, int width, int height) {
    ClientContext context;
    auto stream = stub_->ResizeImage(&context);
    std::string file_name = input_path.substr(input_path.find_last_of("/") + 1);

    // Envia metadados com nome, largura e altura
    ResizeImageRequestChunk req_meta;
    req_meta.mutable_metadata()->set_file_name(file_name);
    req_meta.mutable_metadata()->set_width(width);
    req_meta.mutable_metadata()->set_height(height);
    stream->Write(req_meta);

    // Envia o arquivo em chunks
    std::ifstream input_stream(input_path, std::ios::binary);
    char buffer[1024];
    while (input_stream.read(buffer, sizeof(buffer)) || input_stream.gcount() > 0) {
      ResizeImageRequestChunk req_chunk;
      req_chunk.mutable_chunk()->set_content(buffer, input_stream.gcount());
      stream->Write(req_chunk);
    }
    stream->WritesDone();

    // Recebe a resposta
    ResponseChunk resp;
    std::string output_path = "resized_" + file_name;
    std::ofstream output_stream(output_path, std::ios::binary);
    bool success = false;
    while (stream->Read(&resp)) {
      if (resp.has_metadata()) {
        success = resp.metadata().success();
        std::cout << resp.metadata().status_message() << std::endl;
        if (!success) break;
      } else if (resp.has_chunk()) {
        output_stream.write(resp.chunk().content().data(), resp.chunk().content().size());
      }
    }
    output_stream.close();
    if (success) std::cout << "Salvo em: " << output_path << std::endl;
  }

private:
  // Stub para comunicação com o servidor gRPC
  std::unique_ptr<FileProcessorService::Stub> stub_;
};

// Função principal do cliente
int main() {
  // Cria um canal gRPC para conectar ao servidor
  FileProcessorClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
  std::string choice;
  // Menu interativo para selecionar o serviço
  std::cout << "Escolha o serviço (1: CompressPDF, 2: ConvertToTXT, 3: ConvertImageFormat, 4: ResizeImage): ";
  std::cin >> choice;

  std::string input_path;
  std::cout << "Caminho do arquivo de entrada: ";
  std::cin >> input_path;

  // Chama o serviço selecionado
  if (choice == "1") {
    client.CompressPDF(input_path);
  } else if (choice == "2") {
    client.ConvertToTXT(input_path);
  } else if (choice == "3") {
    std::string output_format;
    std::cout << "Formato de saída (e.g., png, jpg): ";
    std::cin >> output_format;
    client.ConvertImageFormat(input_path, output_format);
  } else if (choice == "4") {
    int width, height;
    std::cout << "Largura desejada: ";
    std::cin >> width;
    std::cout << "Altura desejada: ";
    std::cin >> height;
    client.ResizeImage(input_path, width, height);
  } else {
    std::cout << "Serviço inválido." << std::endl;
  }
  return 0;
}
