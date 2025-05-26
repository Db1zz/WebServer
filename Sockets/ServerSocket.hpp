#ifndef SOCKET_SERVERSOCKET_HPP
#define SOCKET_SERVERSOCKET_HPP

#include "ASocket.hpp"

#include <iostream>

class ServerSocket: public ASocket {
public:
	ServerSocket(int domain, int service, int protocol, int port, u_long interface, int max_connections);
	// ~ServerSocket();
	int start_connection(int socket, struct sockaddr_in address);

private:
	void is_binded(int status);
	void is_listening(int status);

	int _max_connections;
};

#endif  // SOCKET_SERVERSOCKET_HPP
