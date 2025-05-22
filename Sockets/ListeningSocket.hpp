#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP
#include <iostream>
#include "colors.hpp"
#include "ASocket.hpp"

class ListeningSocket: public ASocket {
private:
	int _backlog;
public:
	ListeningSocket(int domain, int service, int protocol, int port, u_long interface);
};
#endif
