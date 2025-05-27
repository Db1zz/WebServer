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
	void init();
	void handle_event(int amount_of_events);

	ServerEvent _event;
};

#endif  // SERVER_SERVER_HPP
