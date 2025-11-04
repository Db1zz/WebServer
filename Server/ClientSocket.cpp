#include "ClientSocket.hpp"

#include "FileDescriptor.hpp"
#include "ServerConfig.hpp"

ClientSocket::ClientSocket(const t_config* server_config)
	: Socket(timer::now()),
	  _server_config(server_config),
	  _server_fd(-1) {
		_socket_type = Socket::CLIENT_SOCKET;
}

ClientSocket::~ClientSocket() {

}

void ClientSocket::set_server_fd(int server_fd) {
	_server_fd = server_fd;
}

int ClientSocket::get_server_fd() {
	return _server_fd;
}