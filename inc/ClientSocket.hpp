#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP
#include <iostream>
#include "colors.hpp"
#include "ASocket.hpp"

class ClientSocket: public ASocket {
private:
public:
	ClientSocket(int domain, int service, int protocol, int port, u_long interface);
	~ClientSocket();
	
	int start_connection(int socket, struct sockaddr_in address);
};
#endif
