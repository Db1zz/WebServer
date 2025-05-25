#include "IServer.hpp"


IServer::IServer(int domain, int service, int protocol, int port, u_long interface, int backlog) {
	_listen_socket = new ListeningSocket(domain, service, protocol, port, interface, backlog);
}

ListeningSocket * IServer::get_socket() {
	return _listen_socket;
}

IServer::~IServer() {
	delete _listen_socket;
}