#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include <netdb.h> /* getnameinfo() */
#include <unistd.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "ServerConfig.hpp"
#include "ServerEvent.hpp"
#include "ServerRequest.hpp"
#include "colors.hpp"

#define SERVER_DEFAULT_ADDR LOCALHOST_ADDR
#define SERVER_DEFAULT_PORT "80"
#define LOCALHOST_ADDR "127.0.0.1"
#define WS_PROTOCOL "HTTP/1.1"
#define IPV4_STR_MAX_SIZE 15
#define IPV4_OCTETS_SIZE 4
#define READ_BUFFER_SIZE 100000

class Status;
class Socket;
class ClientSocket;
class ServerSocket;
class ServerSocketManager;
class ServerLogger;
class CGIFileDescriptor;
class IOServerContext;
class IOServerHandler;

class Server {
   public:
	Server(const std::vector<t_config>& configs, ServerLogger& server_logger);
	~Server();
	void launch();

   private:
	void handle_epoll_event(int amount_of_events);
	void create_server_socket_manager(const std::string& host, int port,
										const t_config& server_config);
	void create_sockets_from_config(const t_config& server_config);
	void create_sockets_from_configs(const std::vector<t_config>& configs);
	void print_debug_addr(const std::string& address, int port);

	std::vector<t_config> _configs;
	ServerEvent _event;
	ServerLogger& _server_logger;
};

#endif // SERVER_SERVER_HPP
