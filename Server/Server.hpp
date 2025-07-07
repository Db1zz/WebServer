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
#include <netdb.h> /* getnameinfo() */
#include <string>
#include <vector>

#define SERVER_DEFAULT_MAX_CONNECTIONS 1024
#define SERVER_DEFAULT_ADDR LOCALHOST_ADDR
#define SERVER_DEFAULT_PORT "80"
#define LOCALHOST_ADDR "127.0.0.1"
#define WS_PROTOCOL "HTTP/1.1"
class Server {
public:
	Server(const std::vector<t_config> &configs);
	~Server();
	Status launch();

private:
	#define IPV4_STR_MAX_SIZE 15
	#define IPV4_OCTETS_SIZE 4

	typedef struct s_event_ctx {
		Socket *socket;
		void *data;

		s_event_ctx(Socket *socket, void *data)
			: socket(socket), data(data) {}
	}	t_event_ctx;

	bool is_a_new_connection(const epoll_event &event);
	Status register_connection_server_event(Socket &new_connection_socket, const t_event_ctx *server_event_ctx);
	Status unregister_connection_server_event(int connection_fd);
	Status set_connection_socket_nonblocking(Socket &socket);
	Status accept_connection(const epoll_event &request_event);
	Status close_connection(const epoll_event &request_event);
	Status handle_event(int amount_of_events);
	Status read_request(const epoll_event &request_event);
	Status request_handler(const epoll_event &request_event);
	Status response_handler(const epoll_event &request_event, const t_request &request);
	std::string response_generator(/* TODO: add args*/);
	Status create_single_server(t_config &server_config, const std::string &host, int port);
	Status create_single_server_with_multiple_addresses(t_config &server_config);
	Status create_multiple_servers();
	void destroy_sockets();
	void print_debug_addr(const std::string &address, int port);
	Status start_servers();

	std::vector<t_config> _configs;
	std::vector<ServerSocket *> _sockets;
	std::vector<t_event_ctx *> _fd_data;
	ServerEvent _event;
	const size_t _server_max_fd_limit;
	size_t _server_amount_of_used_fds;
};

#endif	// SERVER_SERVER_HPP
