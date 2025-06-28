#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include <unistd.h>

#include <iostream>

#include "../Utilities/colors.hpp"
#include "IServer.hpp"
#include "ServerEvent.hpp"
#include "ServerResponse.hpp"
#include "ServerRequest.hpp"
#include "ServerConfig.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <string>
#include <vector>

#define WS_PROTOCOL "HTTP/1.1"
class Server: public IServer {
public:
	Server(std::vector<t_config> configs);
	~Server();
	void launch();

private:
	void init();
	t_request request_parser(std::string request);
	void handle_event(int amount_of_events);
	void accept_new_connection(int new_connection_fd);
	void announce_new_connection(const struct sockaddr &cl_sockaddr, int cl_fd);
	std::string read_request(const epoll_event &request_event);
	Status request_handler(const epoll_event &request_event, t_request &req);
	void response_handler(const epoll_event &request_event, const t_request &request);
	std::string response_generator(/* TODO: add args*/);

	std::vector<t_config> _configs;
	ServerEvent _event;
};

#endif	// SERVER_SERVER_HPP
