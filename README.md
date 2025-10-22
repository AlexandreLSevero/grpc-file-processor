# Serviço de Processamento de Arquivos com gRPC

## Instalação
1. Instale dependências: gRPC, protobuf, Ghostscript, ImageMagick, poppler-utils.
2. Gere código: `protoc --grpc_out=. --cpp_out=. --python_out=. --plugin=protoc-gen-grpc=grpc_cpp_plugin proto/file_processor.proto`
3. Compile servidor/client C++ com CMake.
4. Para Python: `pip install -r client_python/requirements.txt`

## Execução
- Servidor: `docker build -t grpc-server . && docker run -p 50051:50051 grpc-server`
- Clientes: Rode `client_cpp/client` ou `client_python/client.py`

## Testes
Teste com arquivos PDF/imagens. Verifique logs e saídas.
