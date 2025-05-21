#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP
#include <iostream>
#include "colors.hpp"
#include "ASocket.hpp"

class ServerSocket: public ASocket {
private:
public:
	ServerSocket(int domain, int service, int protocol, int port, u_long interface);
	~ServerSocket();
	
	int start_connection(int socket, struct sockaddr_in address);
};
#endif
