#include "Chunk.hpp"
#include "../Utilities/fs.hpp"

Chunk::Chunk() {}

Chunk::~Chunk() {}

std::string Chunk::encode(const std::string& data) {
	std::ostringstream oss;
	oss << std::hex << data.size() << "\r\n" << data << "\r\n";
	return oss.str();
}

std::string Chunk::generate_final_chunk() {
	return "0\r\n\r\n";
}

bool Chunk::is_chunked_response(const std::string& file_path, const t_location* location) {
	size_t file_size = get_file_size(file_path);

	if (location && location->chunked_transfer_encoding)
		return file_size >= location->chunked_threshold;
	return file_size >= DEFAULT_10MB_THRESHOLD;
}

size_t Chunk::get_file_size(const std::string& file_path) {
	std::ifstream file(file_path.c_str(), std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		return 0;
	}
	return static_cast<size_t>(file.tellg());
}

Status Chunk::stream_file_chunked(const std::string& file_path, int client_fd, const t_location* location) {
	size_t chunk_size = DEFAULT_CHUNK_SIZE;
	if (location && location->chunked_size > 0)
		chunk_size = location->chunked_size;
	std::fstream file;
	fs::open_file(file, file_path, std::ios::in | std::ios::binary);
	if (!file.is_open())
		return Status::InternalServerError();
	
	std::vector<char> buffer(chunk_size);
	int chunk_count = 0;
	while (file.good() && !file.eof()) {
		file.read(buffer.data(), chunk_size);
		std::streamsize bytes_read = file.gcount();
		
		if (bytes_read > 0) {
			std::string chunk_data(buffer.data(), bytes_read);
			std::string encoded_chunk = encode(chunk_data);
			
			if (write(client_fd, encoded_chunk.c_str(), encoded_chunk.size()) < 0) {
				file.close();
				return Status::InternalServerError();
			}
			if (++chunk_count % 100 == 0)
				usleep(1000);
		}
	}
	std::string final_chunk = generate_final_chunk();
	if (write(client_fd, final_chunk.c_str(), final_chunk.size()) < 0) {
		file.close();
		return Status::InternalServerError();
	}
	
	file.close();
	return Status::OK();
}

