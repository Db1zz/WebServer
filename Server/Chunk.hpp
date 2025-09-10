#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <string>

class Chunk {
	static std::string encode(const std::string& data);
	static decode(const std::string& chunk);
};

#endif // CHUNK_HPP