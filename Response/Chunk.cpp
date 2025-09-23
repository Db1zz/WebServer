#include "Chunk.hpp"

Chunk::Chunk() {
}

Chunk::~Chunk() {
}

std::string Chunk::encode(const std::string& data) {
	std::ostringstream oss;
	oss << std::hex << data.size() << "\r\n" << data << "\r\n";
	return oss.str();
}

std::string Chunk::decode(const std::string& chunk) {
	return std::string();
}
