#include "ClientSocket.hpp"

ClientSocket::ClientSocket()
	: Socket(), _server_fd(-1) {}

ClientSocket::~ClientSocket() {}

void ClientSocket::set_server_fd(int server_fd) {
	_server_fd = server_fd;
}

int ClientSocket::get_server_fd() {
	return _server_fd;
}

std::string& ClientSocket::get_data_buffer() {
	return _data_buffer;
}