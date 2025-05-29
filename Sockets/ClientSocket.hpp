#ifndef SOCKETS_CLIENTSOCKET_HPP
#define SOCKETS_CLIENTSOCKET_HPP

#include "ASocket.hpp"

#include <iostream>

class ClientSocket: public ASocket {
public:
	ClientSocket(int domain, int service, int protocol, int port, u_long interface);	
	int start_connection();

private:
	void is_connected(int status);
};
#endif  // SOCKETS_CLIENTSOCKET_HPP
