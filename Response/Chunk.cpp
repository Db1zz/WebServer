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


Status Chunk::stream_file_chunked(const std::string& file_path, int client_fd,
									   const t_location* location, size_t& offset,
									   bool& finished, std::string* pending_data,
									   size_t* pending_sent) {
	std::string local_pending;
	size_t local_pending_sent = 0;
	std::string* pend = pending_data ? pending_data : &local_pending;
	size_t* pend_sent = pending_sent ? pending_sent : &local_pending_sent;
	size_t chunk_size = DEFAULT_CHUNK_SIZE;
	if (location && location->chunked_size > 0)
		chunk_size = location->chunked_size;

	std::fstream file;
	fs::open_file(file, file_path, std::ios::in | std::ios::binary);
	if (!file.is_open())
		return Status::InternalServerError();

	if (!pend->empty()) {
		const char* buf = pend->c_str() + *pend_sent;
		size_t to_write = pend->size() - *pend_sent;
		ssize_t w = write(client_fd, buf, to_write);
		if (w < 0) {
			file.close();
			return Status::OK();
		}
		*pend_sent += static_cast<size_t>(w);
		if (*pend_sent < pend->size()) {
			file.close();
			return Status::OK();
		}
		if (*pend == generate_final_chunk()) {
			finished = true;
			pend->clear();
			*pend_sent = 0;
			file.close();
			return Status::OK();
		}
		pend->clear();
		*pend_sent = 0;
	}

	file.seekg(0, std::ios::end);
	std::streamoff file_len = file.tellg();

	if (static_cast<size_t>(file_len) <= offset) {
		std::string final_chunk = generate_final_chunk();
		*pend = final_chunk;
		*pend_sent = 0;
		file.close();
		return Status::OK();
	}

	file.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
	std::vector<char> buffer(chunk_size);
	file.read(buffer.data(), chunk_size);
	std::streamsize bytes_read = file.gcount();

	if (bytes_read <= 0) {
		std::string final_chunk = generate_final_chunk();
		*pend = final_chunk;
		*pend_sent = 0;
		file.close();
		return Status::OK();
	}

	std::string chunk_data(buffer.data(), bytes_read);
	std::string encoded_chunk = encode(chunk_data);

	ssize_t w = write(client_fd, encoded_chunk.c_str(), encoded_chunk.size());
	if (w < 0) {
		*pend = encoded_chunk;
		*pend_sent = 0;
		file.close();
		return Status::OK();
	}

	if (static_cast<size_t>(w) < encoded_chunk.size()) {
		*pend = encoded_chunk;
		*pend_sent = static_cast<size_t>(w);
		file.close();
		return Status::OK();
	}

	offset += static_cast<size_t>(bytes_read);

	file.seekg(0, std::ios::end);
	std::streamoff new_len = file.tellg();
	if (static_cast<size_t>(new_len) <= offset) {
		*pend = generate_final_chunk();
		*pend_sent = 0;
	}

	file.close();
	finished = false;
	return Status::OK();
}

