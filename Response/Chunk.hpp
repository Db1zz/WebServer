#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <sstream>
#include <string>

class Chunk {
   public:
	Chunk();
	~Chunk();
	static std::string encode(const std::string& data);
	static std::string generate_final_chunk();
};

#endif // CHUNK_HPP