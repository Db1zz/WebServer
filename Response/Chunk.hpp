#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <sstream>
#include <string>

class Chunk {
   public:
	Chunk();
	~Chunk();
	static std::string encode(const std::string& data);
};

#endif // CHUNK_HPP