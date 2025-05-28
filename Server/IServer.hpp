#ifndef SERVER_ISERVER_HPP
#define SERVER_ISERVER_HPP

#include "colors.hpp"
#include "ServerSocket.hpp"

#include <iostream>

class IServer {
public:
	IServer(int domain, int service, int protocol, int port, u_long interface, int backlog);
	virtual ~IServer();
	virtual void launch() = 0;
	ServerSocket *get_socket();

private:
	ServerSocket _server_socket;
};

#endif  // SERVER_ISERVER_HPP
