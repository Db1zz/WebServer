#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <unistd.h>
#include "../Utilities/status.hpp"
#include "../Server/ServerConfig.hpp"

class Chunk {
   public:
	Chunk();
	~Chunk();
	
	static std::string encode(const std::string& data);
	static std::string generate_final_chunk();
	
	static bool is_chunked_response(const std::string& file_path, const t_location* location);
	static size_t get_file_size(const std::string& file_path);
	static Status stream_file_chunked(const std::string& file_path, int client_fd, const t_location* location);
	
   private:
	static const size_t DEFAULT_10MB_THRESHOLD = 10 * 1024 * 1024;
	static const size_t DEFAULT_CHUNK_SIZE = 8192;
};

#endif // CHUNK_HPP