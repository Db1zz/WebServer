#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP
#include <iostream>
#include "colors.hpp"
#include "ASocket.hpp"
#include "ServerSocket.hpp"

class ListeningSocket: public ServerSocket {
private:
	int _backlog;
	int _listening;
public:
	ListeningSocket(int domain, int service, int protocol, int port, u_long interface, int backlog);
	void start_listening();
};
#endif
