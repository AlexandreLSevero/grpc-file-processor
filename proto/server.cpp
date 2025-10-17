#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "file_processor.grpc.pb.h"
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;
using file_processor::FileProcessorService;
using file_processor::ClientChunk;
using file_processor::ServerChunk;
using file_processor::FileMetadata;
using file_processor::StatusMessage;

class FileProcessorServiceImpl final : public FileProcessorService::Service {
private:
  void Log(const std::string& level, const std::string& service_name, const std::string& file_name, const std::string& message) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_r(&now_c, &now_tm);
    char timestamp[26];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &now_tm);
    std::ofstream log_file("server.log", std::ios::app);
    if (log_file.is_open()) {
      log_file << "[" << timestamp << "] " << level << " - Service: " << service_name << ", File: " << file_name << ", Message: " << message << std::endl;
      log_file.close();
    }
    std::cout << "[" << timestamp << "] " << level << " - Service: " << service_name << ", File: " << file_name << ", Message: " << message << std::endl;
  }

  void LogSuccess(const std::string& service_name, const std::string& file_name, const std::string& message) {
    Log("SUCCESS", service_name, file_name, message);
  }

  void LogError(const std::string& service_name, const std::string& file_name, const std::string& message) {
    Log("ERROR", service_name, file_name, message);
  }

  Status ProcessCompressPDF(ServerContext* context, ServerReaderWriter<ServerChunk, ClientChunk>* stream) {
    return ProcessPDF(stream, "CompressPDF", "gs -sDEVICE=pdfwrite -dCompatibilityLevel=1.4 -dPDFSETTINGS=/ebook -dNOPAUSE -dQUIET -dBATCH -sOutputFile=", "compressed_");
  }

  Status ProcessConvertToTXT(ServerContext* context, ServerReaderWriter<ServerChunk, ClientChunk>* stream) {
    return ProcessPDF(stream, "ConvertToTXT", "pdftotext ", "extracted.txt");
  }

  Status ProcessConvertImageFormat(ServerContext* context, ServerReaderWriter<ServerChunk, ClientChunk>* stream) {
    return ProcessImage(stream, "ConvertImageFormat", "convert ", true);
  }

  Status ProcessResizeImage(ServerContext* context, ServerReaderWriter<ServerChunk, ClientChunk>* stream) {
    return ProcessImage(stream, "ResizeImage", "convert ", false);
  }

  Status ProcessPDF(ServerReaderWriter<ServerChunk, ClientChunk>* stream, const std::string& service_name, const std::string& cmd_prefix, const std::string& output_prefix) {
    std::string file_name;
    std::string input_file_path;
    std::string output_file_path;
    std::ofstream input_file_stream;
    bool metadata_received = false;
    ClientChunk chunk;
    while (stream->Read(&chunk)) {
      if (chunk.data().has_metadata()) {
        if (metadata_received) {
          LogError(service_name, "", "Metadata duplicado.");
          SendError(stream, "Metadata duplicado.");
          return Status::OK;
        }
        metadata_received = true;
        const FileMetadata& meta = chunk.data().metadata();
        file_name = meta.file_name();
        input_file_path = "/tmp/input_" + file_name;
        if (service_name == "ConvertToTXT") {
          output_file_path = "/tmp/output.txt";
        } else {
          output_file_path = "/tmp/output_" + file_name;
        }
        input_file_stream.open(input_file_path, std::ios::binary);
        if (!input_file_stream) {
          LogError(service_name, file_name, "Falha ao criar arquivo temporário.");
          SendError(stream, "Erro ao criar arquivo temporário.");
          return Status::OK;
        }
      } else if (chunk.data().has_content()) {
        if (!metadata_received) {
          LogError(service_name, "", "Chunks antes de metadata.");
          SendError(stream, "Chunks antes de metadata.");
          return Status::OK;
        }
        input_file_stream.write(chunk.data().content().c_str(), chunk.data().content().size());
      }
    }
    if (input_file_stream.is_open()) input_file_stream.close();
    if (!metadata_received) {
      LogError(service_name, "", "Metadata não recebido.");
      SendError(stream, "Metadata não recebido.");
      return Status::OK;
    }
    std::string command = cmd_prefix + output_file_path + " " + input_file_path;
    int result = system(command.c_str());
    return SendResult(stream, service_name, file_name, result, input_file_path, output_file_path, output_prefix + file_name);
  }

  Status ProcessImage(ServerReaderWriter<ServerChunk, ClientChunk>* stream, const std::string& service_name, const std::string& cmd_prefix, bool is_convert_format) {
    std::string file_name;
    std::string input_file_path;
    std::string output_file_path;
    std::string ext;
    std::ofstream input_file_stream;
    bool metadata_received = false;
    ClientChunk chunk;
    std::string output_format;
    int32_t width = 0;
    int32_t height = 0;
    while (stream->Read(&chunk)) {
      if (chunk.data().has_metadata()) {
        if (metadata_received) {
          LogError(service_name, "", "Metadata duplicado.");
          SendError(stream, "Metadata duplicado.");
          return Status::OK;
        }
        metadata_received = true;
        const FileMetadata& meta = chunk.data().metadata();
        file_name = meta.file_name();
        input_file_path = "/tmp/input_" + file_name;
        if (is_convert_format) {
          output_format = meta.output_format();
          if (output_format.empty()) {
            LogError(service_name, file_name, "Formato de saída não fornecido.");
            SendError(stream, "Formato de saída não fornecido.");
            return Status::OK;
          }
          output_file_path = "/tmp/output." + output_format;
        } else {
          width = meta.width();
          height = meta.height();
          if (width <= 0 || height <= 0) {
            LogError(service_name, file_name, "Dimensões inválidas.");
            SendError(stream, "Dimensões inválidas.");
            return Status::OK;
          }
          size_t dot = file_name.rfind('.');
          ext = (dot != std::string::npos) ? file_name.substr(dot) : ".jpg";
          output_file_path = "/tmp/output" + ext;
        }
        input_file_stream.open(input_file_path, std::ios::binary);
        if (!input_file_stream) {
          LogError(service_name, file_name, "Falha ao criar arquivo temporário.");
          SendError(stream, "Erro ao criar arquivo temporário.");
          return Status::OK;
        }
      } else if (chunk.data().has_content()) {
        if (!metadata_received) {
          LogError(service_name, "", "Chunks antes de metadata.");
          SendError(stream, "Chunks antes de metadata.");
          return Status::OK;
        }
        input_file_stream.write(chunk.data().content().c_str(), chunk.data().content().size());
      }
    }
    if (input_file_stream.is_open()) input_file_stream.close();
    if (!metadata_received) {
      LogError(service_name, "", "Metadata não recebido.");
      SendError(stream, "Metadata não recebido.");
      return Status::OK;
    }
    std::string command = cmd_prefix + input_file_path;
    if (is_convert_format) {
      command += " " + output_file_path;
    } else {
      command += " -resize " + std::to_string(width) + "x" + std::to_string(height) + " " + output_file_path;
    }
    int result = system(command.c_str());
    std::string output_name = is_convert_format ? "converted." + output_format : "resized" + ext;
    return SendResult(stream, service_name, file_name, result, input_file_path, output_file_path, output_name);
  }

  void SendError(ServerReaderWriter<ServerChunk, ClientChunk>* stream, const std::string& message) {
    ServerChunk resp;
    StatusMessage* sm = resp.mutable_data().mutable_status();
    sm->set_success(false);
    sm->set_message(message);
    stream->Write(resp);
  }

  Status SendResult(ServerReaderWriter<ServerChunk, ClientChunk>* stream, const std::string& service_name, const std::string& file_name, int result, const std::string& input_path, const std::string& output_path, const std::string& output_name) {
    if (result != 0) {
      LogError(service_name, file_name, "Falha no processamento. Código: " + std::to_string(result));
      SendError(stream, "Falha no processamento.");
      std::remove(input_path.c_str());
      return Status::OK;
    }
    LogSuccess(service_name, file_name, "Processamento bem-sucedido.");
    ServerChunk resp_status;
    StatusMessage* sm = resp_status.mutable_data().mutable_status();
    sm->set_success(true);
    sm->set_message("Sucesso");
    sm->set_output_file_name(output_name);
    stream->Write(resp_status);
    std::ifstream output_stream(output_path, std::ios::binary);
    if (!output_stream) {
      LogError(service_name, file_name, "Falha ao abrir output.");
      SendError(stream, "Erro ao abrir output.");
      std::remove(input_path.c_str());
      std::remove(output_path.c_str());
      return Status::OK;
    }
    char buffer[1024];
    while (output_stream.read(buffer, sizeof(buffer))) {
      ServerChunk resp_chunk;
      resp_chunk.mutable_data().set_content(buffer, output_stream.gcount());
      stream->Write(resp_chunk);
    }
    if (output_stream.gcount() > 0) {
      ServerChunk resp_chunk;
      resp_chunk.mutable_data().set_content(buffer, output_stream.gcount());
      stream->Write(resp_chunk);
    }
    output_stream.close();
    std::remove(input_path.c_str());
    std::remove(output_path.c_str());
    return Status::OK;
  }

public:
  Status CompressPDF(ServerContext* context, ServerReaderWriter<ServerChunk, ClientChunk>* stream) override {
    return ProcessCompressPDF(context, stream);
  }

  Status ConvertToTXT(ServerContext* context, ServerReaderWriter<ServerChunk, ClientChunk>* stream) override {
    return ProcessConvertToTXT(context, stream);
  }

  Status ConvertImageFormat(ServerContext* context, ServerReaderWriter<ServerChunk, ClientChunk>* stream) override {
    return ProcessConvertImageFormat(context, stream);
  }

  Status ResizeImage(ServerContext* context, ServerReaderWriter<ServerChunk, ClientChunk>* stream) override {
    return ProcessResizeImage(context, stream);
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  FileProcessorServiceImpl service;
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Servidor gRPC ouvindo em " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();
  return 0;
}
