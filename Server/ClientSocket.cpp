#include "ClientSocket.hpp"
ClientSocket::ClientSocket() : Socket(), _request_data(NULL), _server_fd(-1) {
}

ClientSocket::~ClientSocket() {
}

void ClientSocket::set_server_fd(int server_fd) {
	_server_fd = server_fd;
}

int ClientSocket::get_server_fd() {
	return _server_fd;
}

void ClientSocket::reset_request() {
	if (_request_data) {
		delete _request_data;
		_request_data = NULL;
	}
}

t_request* ClientSocket::get_request_data() {
	if (_request_data == NULL) {
		_request_data = new t_request;
	}
	return _request_data;
}