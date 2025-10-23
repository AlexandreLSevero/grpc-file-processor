#ifndef FILE_PROCESSOR_SERVICE_IMPL_H
#define FILE_PROCESSOR_SERVICE_IMPL_H

// Inclui biblioteca gRPC e o arquivo gerado pelo protobuf
#include <grpcpp/grpcpp.h>
#include "file_processor.grpc.pb.h"

// Usa o namespace gerado pelo protobuf
using file_processor::FileProcessorService;

// Classe que implementa o serviço gRPC
class FileProcessorServiceImpl final : public FileProcessorService::Service {
public:
  // Declaração dos métodos para cada serviço gRPC
  // Cada método recebe um stream de entrada e retorna um stream de saída
  grpc::Status CompressPDF(grpc::ServerContext* context, grpc::ServerReaderWriter<file_processor::ResponseChunk, file_processor::CompressRequestChunk>* stream) override;
  grpc::Status ConvertToTXT(grpc::ServerContext* context, grpc::ServerReaderWriter<file_processor::ResponseChunk, file_processor::ConvertToTXTRequestChunk>* stream) override;
  grpc::Status ConvertImageFormat(grpc::ServerContext* context, grpc::ServerReaderWriter<file_processor::ResponseChunk, file_processor::ConvertImageFormatRequestChunk>* stream) override;
  grpc::Status ResizeImage(grpc::ServerContext* context, grpc::ServerReaderWriter<file_processor::ResponseChunk, file_processor::ResizeImageRequestChunk>* stream) override;

// Funções auxiliares para registrar logs de sucesso e logs de erro
private:
  void LogSuccess(const std::string& service_name, const std::string& file_name, const std::string& message);
  void LogError(const std::string& service_name, const std::string& file_name, const std::string& message);
};

#endif
