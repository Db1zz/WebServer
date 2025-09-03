#include "ClientSocket.hpp"
ClientSocket::ClientSocket()
	: Socket(),
	  _request_content_length(REQUEST_BUFFER_IS_EMPTY),
	  _request_ready(false),
	  _server_fd(-1) {
}

ClientSocket::~ClientSocket() {
}

void ClientSocket::set_server_fd(int server_fd) {
	_server_fd = server_fd;
}

int ClientSocket::get_server_fd() {
	return _server_fd;
}

std::string& ClientSocket::get_request_buffer() {
	return _request_buffer;
}

void ClientSocket::reset_request_buffer() {
	_request_ready = false;
	_request_buffer.clear();
	_request_content_length = REQUEST_BUFFER_IS_EMPTY;
}

bool ClientSocket::is_request_ready() {
	return _request_ready;
}

void ClientSocket::set_request_ready() {
	_request_ready = true;
}

size_t ClientSocket::get_request_content_length() {
	return _request_content_length;
}

void ClientSocket::set_request_content_length(size_t request_content_length) {
	_request_content_length = request_content_length;
}

// #include "ClientSocket.hpp"

// ClientSocket::ClientSocket() : Socket(), _request(NULL), _server_fd(-1) {
// }

// ClientSocket::~ClientSocket() {
// }

// void ClientSocket::set_server_fd(int server_fd) {
// 	_server_fd = server_fd;
// }

// int ClientSocket::get_server_fd() {
// 	return _server_fd;
// }

// t_request* ClientSocket::get_request() {
// 	if (!_request) {
// 		_request = new t_request;
// 	}
// 	return _request;
// }

// void ClientSocket::reset_request() {
// 	if (_request) {
// 		delete _request;
// 		_request = NULL;
// 	}
// }