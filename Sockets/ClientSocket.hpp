#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP
#include <iostream>
#include "colors.hpp"
#include "ASocket.hpp"

class ClientSocket: public ASocket {
public:
	ClientSocket(int domain, int service, int protocol, int port, u_long interface);	
	int start_connection(int socket, struct sockaddr_in address);
};
#endif
