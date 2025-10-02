#include "ClientSocket.hpp"

#include "FileDescriptor.hpp"

ClientConnectionContext::ClientConnectionContext()
	: request(), parser(&request), state(ConnectionState::IDLE), cgi_started(false) {
}

void ClientConnectionContext::reset() {
	request = t_request();
	parser = ServerRequestParser(&request);
	state = ConnectionState::IDLE;
	cgi_started = false;
	buffer.clear();
}

ClientSocket::ClientSocket() : Socket(), _server_fd(FileDescriptor::SocketFD, -1) {
	_socket_type = Socket::CLIENT_SOCKET;
}

ClientSocket::~ClientSocket() {
}

void ClientSocket::set_server_fd(int server_fd) {
	_server_fd.set_fd(server_fd);
}

int ClientSocket::get_server_fd() {
	return _server_fd.get_fd();
}

void ClientSocket::reset_connection_context() {
	_connection_context.reset();
}

ClientConnectionContext* ClientSocket::get_connection_context() {
	return &_connection_context;
}