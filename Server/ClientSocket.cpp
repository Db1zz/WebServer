#include "ClientSocket.hpp"

#include "FileDescriptor.hpp"
#include "ServerConfig.hpp"

ClientSocket::ClientSocket(const t_config* server_config)
	: Socket(),
	  _server_config(server_config),
	  _connection_context(server_config),
	  _server_fd(FileDescriptor::SocketFD, -1) {
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

ConnectionContext* ClientSocket::get_connection_context() {
	return &_connection_context;
}