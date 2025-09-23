#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <sstream>
#include <string>

class Chunk {
   public:
	Chunk::Chunk() {}
	Chunk::~Chunk() {}
	static std::string encode(const std::string& data);
	static std::string decode(const std::string& chunk);
};

#endif // CHUNK_HPP