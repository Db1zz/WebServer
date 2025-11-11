#ifndef SERVER_SERVER_SOCKET_HPP
#define SERVER_SERVER_SOCKET_HPP

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#include <iostream>
#include <map>

#include "Socket.hpp"

class ClientSocket;
class ServerSocket : public Socket {
   public:
	ServerSocket(const std::string& host, int port);
	void open_socket();
	void accept_connection(ClientSocket& empty_client_socket);

   private:
	void create_server_socket();
	// const int _max_connections = 1024;
};

#endif // SERVER_SERVER_SOCKET_HPP
