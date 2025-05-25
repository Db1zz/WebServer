#ifndef ISERVER_HPP
#define ISERVER_HPP
#include <iostream>
#include "colors.hpp"
#include "../Sockets/ListeningSocket.hpp"

class IServer {
private:
	ListeningSocket *_listen_socket;
	virtual void server_accept() = 0;
	virtual void server_process_handle() = 0;
	virtual void server_respond() = 0;
public:
	IServer(int domain, int service, int protocol, int port, u_long interface, int backlog);
	virtual ~IServer();
	virtual void launch() = 0;
	ListeningSocket *get_socket();
};

#endif
