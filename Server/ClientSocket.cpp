#include "ClientSocket.hpp"

ClientSocket::ClientSocket() : Socket(), _request_ready(false), _server_fd(-1) {
}

ClientSocket::~ClientSocket() {
}

void ClientSocket::set_server_fd(int server_fd) {
	_server_fd = server_fd;
}

int ClientSocket::get_server_fd() {
	return _server_fd;
}

std::string& ClientSocket::get_data_buffer() {
	return _request_buffer;
}

void ClientSocket::reset_request_buffer() {
	_request_ready = false;
	_request_buffer.clear();
}

bool ClientSocket::is_request_ready() {
	return _request_ready;
}