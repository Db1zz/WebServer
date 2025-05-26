#include "IServer.hpp"

IServer::IServer(int domain, int service, int protocol, int port, u_long interface, int backlog) {
	_server_socket = new ServerSocket(domain, service, protocol, port, interface, backlog);
}

ServerSocket* IServer::get_socket() {
	return _server_socket;
}

IServer::~IServer() {
	delete _server_socket;
}