#ifndef SERVER_HPP
#define SERVER_HPP
#include <iostream>
#include "colors.hpp"
#include "IServer.hpp"
#include "../Sockets/ListeningSocket.hpp"
#include <unistd.h>

class Server: public IServer {
private:
	char _buffer[40000]; //change to reading to file later
	int _new_socket;
	void server_accept();
	void server_process_handle();
	void server_respond();
public:
	Server();
	void launch();
};

#endif
