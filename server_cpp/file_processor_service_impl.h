#ifndef FILE_PROCESSOR_SERVICE_IMPL_H
#define FILE_PROCESSOR_SERVICE_IMPL_H

#include <grpcpp/grpcpp.h>
#include "file_processor.grpc.pb.h"

using file_processor::FileProcessorService;

class FileProcessorServiceImpl final : public FileProcessorService::Service {
public:
  grpc::Status CompressPDF(grpc::ServerContext* context, grpc::ServerReaderWriter<file_processor::ResponseChunk, file_processor::CompressRequestChunk>* stream) override;
  grpc::Status ConvertToTXT(grpc::ServerContext* context, grpc::ServerReaderWriter<file_processor::ResponseChunk, file_processor::ConvertToTXTRequestChunk>* stream) override;
  grpc::Status ConvertImageFormat(grpc::ServerContext* context, grpc::ServerReaderWriter<file_processor::ResponseChunk, file_processor::ConvertImageFormatRequestChunk>* stream) override;
  grpc::Status ResizeImage(grpc::ServerContext* context, grpc::ServerReaderWriter<file_processor::ResponseChunk, file_processor::ResizeImageRequestChunk>* stream) override;

private:
  void LogSuccess(const std::string& service_name, const std::string& file_name, const std::string& message);
  void LogError(const std::string& service_name, const std::string& file_name, const std::string& message);
};

#endif
