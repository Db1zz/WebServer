#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include "colors.hpp"
#include "IServer.hpp"

#include <iostream>
#include <unistd.h>

class Server: public IServer {
public:
	Server();
	void launch();

private:
	void server_accept();
	void server_process_handle();
	void server_respond();

	char _buffer[40000]; //change to reading to file later
	int _new_socket;
};

#endif  // SERVER_SERVER_HPP
