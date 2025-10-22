Instalação
# 1. Instalar Dependências no Host (para desenvolvimento)

## Ubuntu:
sudo apt update
sudo apt install -y build-essential cmake git libgrpc-dev protobuf-compiler grpc-tools ghostscript poppler-utils imagemagick python3-pip

## Python (para cliente Python):
cd client_python
pip install -r requirements.txt

OBS:O arquivo client_python/requirements.txt contém:
grpcio
grpcio-tools

# 2. Gerar Código gRPC:

protoc --grpc_out=server_cpp --cpp_out=server_cpp --python_out=client_python \
       --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` proto/file_processor.proto


# 3. Compilar Servidor e Cliente C++

## Servidor C++
cd server_cpp

mkdir build && cd build

cmake .. && make

## Cliente C++

cd client_cpp

mkdir build && cd build

cmake .. && make


# Containerização com Docker

## Construir a Imagem Docker
docker build -t grpc-server .

## Executar o Container
docker run -p 50051:50051 -v $(pwd):/app grpc-server

# Container
docker ps  # Encontre o ID do container

docker stop <container_id>


# Execução

# 1. Iniciar o Servidor

## Via Docker
docker run -p 50051:50051 -v $(pwd):/app grpc-server

## Diretamente no host
cd server_cpp/build

./server

# 2. Executar Clientes

## Cliente python:
cd client_python

python client.py

## Cliente C++
cd client_cpp/build

./client
