#ifndef SERVER_SERVER_SOCKET_HPP
#define SERVER_SERVER_SOCKET_HPP

#include "Socket.hpp"

#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

class ServerSocket : public Socket {
public:
	ServerSocket(const std::string &host, int port);
	Status listen_for_connections();
	Status accept_connection(Socket &empty_socket);

private:
	void is_socket_created(int socket_fd);
	// const int _max_connections = 1024;
};

#endif  // SERVER_SERVER_SOCKET_HPP
