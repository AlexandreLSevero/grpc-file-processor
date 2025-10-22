#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>
#include "file_processor_service_impl.h"

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using namespace file_processor;

void FileProcessorServiceImpl::LogSuccess(const std::string& service_name, const std::string& file_name, const std::string& message) {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm;
  localtime_r(&now_c, &now_tm);
  char timestamp[26];
  std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &now_tm);
  std::ofstream log_file("server.log", std::ios::app);
  if (log_file.is_open()) {
    log_file << "[" << timestamp << "] SUCCESS - Service: " << service_name << ", File: " << file_name << ", Message: " << message << std::endl;
    log_file.close();
  }
  std::cout << "[" << timestamp << "] SUCCESS - Service: " << service_name << ", File: " << file_name << ", Message: " << message << std::endl;
}

void FileProcessorServiceImpl::LogError(const std::string& service_name, const std::string& file_name, const std::string& message) {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm;
  localtime_r(&now_c, &now_tm);
  char timestamp[26];
  std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &now_tm);
  std::ofstream log_file("server.log", std::ios::app);
  if (log_file.is_open()) {
    log_file << "[" << timestamp << "] ERROR - Service: " << service_name << ", File: " << file_name << ", Message: " << message << std::endl;
    log_file.close();
  }
  std::cerr << "[" << timestamp << "] ERROR - Service: " << service_name << ", File: " << file_name << ", Message: " << message << std::endl;
}

Status FileProcessorServiceImpl::CompressPDF(ServerContext* context, ServerReaderWriter<ResponseChunk, CompressRequestChunk>* stream) {
  CompressRequestChunk request_chunk;
  std::string file_name;
  std::string input_file_path;
  bool has_metadata = false;
  std::ofstream input_file_stream;

  while (stream->Read(&request_chunk)) {
    if (request_chunk.has_metadata()) {
      if (has_metadata) {
        LogError("CompressPDF", file_name, "Múltiplos metadados recebidos.");
        return Status(grpc::INVALID_ARGUMENT, "Múltiplos metadados");
      }
      has_metadata = true;
      file_name = request_chunk.metadata().file_name();
      input_file_path = "/tmp/" + file_name;
      input_file_stream.open(input_file_path, std::ios::binary);
      if (!input_file_stream) {
        LogError("CompressPDF", file_name, "Falha ao criar arquivo temporário.");
        return Status(grpc::INTERNAL, "Erro ao criar arquivo temporário");
      }
    } else if (request_chunk.has_chunk()) {
      if (!has_metadata) {
        LogError("CompressPDF", "", "Chunks recebidos sem metadados.");
        return Status(grpc::INVALID_ARGUMENT, "Sem metadados");
      }
      input_file_stream.write(request_chunk.chunk().content().data(), request_chunk.chunk().content().size());
    }
  }
  if (input_file_stream.is_open()) input_file_stream.close();

  if (!has_metadata) {
    LogError("CompressPDF", "", "Nenhum metadado recebido.");
    return Status(grpc::INVALID_ARGUMENT, "Sem metadados");
  }

  std::string output_file_path = "/tmp/compressed_" + file_name;
  std::string command = "gs -sDEVICE=pdfwrite -dCompatibilityLevel=1.4 -dPDFSETTINGS=/ebook -dNOPAUSE -dQUIET -dBATCH -sOutputFile=" + output_file_path + " " + input_file_path;
  int result = system(command.c_str());
  if (result != 0) {
    LogError("CompressPDF", file_name, "Falha na compressão. Código: " + std::to_string(result));
    ResponseChunk resp;
    resp.mutable_metadata()->set_success(false);
    resp.mutable_metadata()->set_status_message("Falha na compressão");
    stream->Write(resp);
    unlink(input_file_path.c_str());
    return Status::OK;
  }

  ResponseChunk resp_meta;
  resp_meta.mutable_metadata()->set_success(true);
  resp_meta.mutable_metadata()->set_file_name("compressed_" + file_name);
  resp_meta.mutable_metadata()->set_status_message("Sucesso");
  stream->Write(resp_meta);

  std::ifstream output_stream(output_file_path, std::ios::binary);
  char buffer[1024];
  while (output_stream.read(buffer, sizeof(buffer)) || output_stream.gcount() > 0) {
    ResponseChunk resp_chunk;
    resp_chunk.mutable_chunk()->set_content(buffer, output_stream.gcount());
    stream->Write(resp_chunk);
  }
  output_stream.close();

  unlink(input_file_path.c_str());
  unlink(output_file_path.c_str());
  LogSuccess("CompressPDF", file_name, "Compressão concluída com sucesso.");
  return Status::OK;
}

// Implementações similares para os outros serviços (ConvertToTXT, ConvertImageFormat, ResizeImage).
// Para brevidade, aqui está o ConvertToTXT como exemplo; os outros seguem o mesmo padrão.

Status FileProcessorServiceImpl::ConvertToTXT(ServerContext* context, ServerReaderWriter<ResponseChunk, ConvertToTXTRequestChunk>* stream) {
  // Código similar ao CompressPDF, mas com comando: "pdftotext " + input_file_path + " " + output_file_path
  // output_file_path = "/tmp/" + file_name + ".txt"
  // Ajuste o file_name na resposta para terminar com .txt
  // ... (implemente analogamente)
}

// Para ConvertImageFormat:
// Comando: "convert " + input_file_path + " /tmp/output." + metadata.output_format()
// file_name na resposta: "converted." + output_format

// Para ResizeImage:
// Comando: "convert " + input_file_path + " -resize " + std::to_string(metadata.width()) + "x" + std::to_string(metadata.height()) + " " + output_file_path
// output_file_path = "/tmp/resized_" + file_name
