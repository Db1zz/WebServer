#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include <unistd.h>

#include <iostream>

#include "colors.hpp"
#include "ServerEvent.hpp"
#include "ServerResponse.hpp"
#include "ServerConfig.hpp"
#include "ServerSocket.hpp"
#include "ServerRequest.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <string>
#include <vector>

#define SERVER_DEFAULT_ADDR LOCALHOST_ADDR
#define SERVER_DEFAULT_PORT "80"
#define LOCALHOST_ADDR "127.0.0.1"
#define WS_PROTOCOL "HTTP/1.1"
class Server {
public:
	Server(std::vector<t_config> configs);
	~Server();
	Status launch();

private:
	void init();
	Status handle_event(int amount_of_events);
	Status accept_new_connection(int new_connection_fd);
	Status announce_new_connection(const struct sockaddr &cl_sockaddr, int cl_fd);
	std::vector<std::string> read_request(const epoll_event &request_event);
	std::vector<std::string> request_handler(const epoll_event &request_event);
	Status response_handler(const epoll_event &request_event, const t_request &request);
	std::string response_generator(/* TODO: add args*/);
	Status create_sockets_from_configs();
	void destroy_sockets();
	void print_debug_addr(const std::string &address, const std::string &port);
	void set_default_host_and_port_if_needed(t_config &config);

	std::vector<t_config> _configs;
	std::vector<ServerSocket *> _sockets;
	ServerEvent _event;
};

#endif	// SERVER_SERVER_HPP
