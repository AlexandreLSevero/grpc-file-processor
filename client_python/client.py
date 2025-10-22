import grpc
import file_processor_pb2
import file_processor_pb2_grpc
import os

def compress_pdf(stub, input_path):
    def request_iterator():
        file_name = os.path.basename(input_path)
        yield file_processor_pb2.CompressRequestChunk(metadata=file_processor_pb2.CompressMetadata(file_name=file_name))
        with open(input_path, 'rb') as f:
            while True:
                chunk = f.read(1024)
                if not chunk:
                    break
                yield file_processor_pb2.CompressRequestChunk(chunk=file_processor_pb2.Chunk(content=chunk))

    response_stream = stub.CompressPDF(request_iterator())
    output_path = "compressed_" + os.path.basename(input_path)
    with open(output_path, 'wb') as output_file:
        success = False
        for resp in response_stream:
            if resp.HasField('metadata'):
                success = resp.metadata.success
                print(resp.metadata.status_message)
                if not success:
                    break
            elif resp.HasField('chunk'):
                output_file.write(resp.chunk.content)
    if success:
        print(f"Salvo em: {output_path}")

# Funções similares para outros serviços (use input() para params)

def run_client():
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = file_processor_pb2_grpc.FileProcessorServiceStub(channel)
        service = input("Escolha o serviço (compress_pdf, convert_to_txt, convert_image_format, resize_image): ")
        input_path = input("Caminho do arquivo de entrada: ")
        if service == "compress_pdf":
            compress_pdf(stub, input_path)
        # ... adicione ifs para outros

if __name__ == '__main__':
    run_client()
