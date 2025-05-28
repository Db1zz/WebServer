#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include <unistd.h>

#include <iostream>

#include "../Utilities/colors.hpp"
#include "IServer.hpp"
#include "ServerEvent.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <string>
#include <vector>

class Server: public IServer {
public:
	Server();
	~Server();
	void launch();

private:
	void init();
	void handle_event(int amount_of_events);
	void accept_new_connection(int new_connection_fd);
	void announce_new_connection(const struct sockaddr &cl_sockaddr, int cl_fd);
	std::vector<std::string> read_request(const epoll_event &request_event);
	std::vector<std::string> request_handler(const epoll_event &request_event);
	void response_handler(const epoll_event &request_event);
	std::string response_generator(/* TODO: add args*/);

	ServerEvent _event;
};

#endif	// SERVER_SERVER_HPP
