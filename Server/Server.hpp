#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include "colors.hpp"
#include "IServer.hpp"
#include "ServerEvent.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>

class Server: public IServer {
public:
	Server();
	~Server();
	void launch();

private:
	ServerEvent event;

	void init();

	void accept();
	void process_handle();
	void respond();
};

#endif  // SERVER_SERVER_HPP
