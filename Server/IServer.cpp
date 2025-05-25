#include "IServer.hpp"


IServer::IServer(int domain, int port, int service, int protocol, int port, u_long interface, int backlog) {
	_socket = new ListeningSocket(domain, service, protocol, port, interface, backlog);
}

ListeningSocket * IServer::get_socket() {
	return _socket;
}

IServer::~IServer() {
	delete _socket;
}