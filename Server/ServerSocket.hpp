#ifndef SERVER_SERVER_SOCKET_HPP
#define SERVER_SERVER_SOCKET_HPP

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#include <iostream>
#include <map>

#include "ClientSocket.hpp"
#include "Socket.hpp"

class ServerSocket : public Socket {
   public:
	ServerSocket(const std::string& host, int port);
	Status open_socket();
	Status accept_connection(ClientSocket& empty_client_socket);

   private:
	Status create_server_socket();
	// const int _max_connections = 1024;
};

#endif // SERVER_SERVER_SOCKET_HPP
