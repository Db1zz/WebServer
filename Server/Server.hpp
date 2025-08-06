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

class Status;
class Socket;
class ClientSocket;
class ServerSocket;
class ServerSocketManager;
class ServerLogger;

class Server {
   public:
	Server(const std::vector<t_config>& configs, ServerLogger& server_logger);
	~Server();
	Status launch();

   private:
	bool is_a_new_connection(const epoll_event& event);
	t_request request_parser(std::string request);
	Status handle_new_connection_event(const epoll_event& connection_event);
	Status handle_request_event(const epoll_event& request_event);
	Status handle_event(int amount_of_events);
	Status read_request(const ClientSocket* client_socket, std::string& result);
	Status request_handler(const ClientSocket* client_socket, t_request& req);
	Status response_handler(const ClientSocket* client_socket, const t_request& request);
	Status create_server_socket_manager(const std::string& host, int port);
	Status create_sockets_from_config(const t_config& server_config);
	Status create_sockets_from_configs(const std::vector<t_config>& configs);
	void print_debug_addr(const std::string& address, int port);

	Status find_server_socket_manager(int server_socket_fd,
									  std::map<int, ServerSocketManager*>::iterator& search_result);
	void destroy_all_server_socket_managers();

	std::vector<t_config> _configs;
	std::map<int, ServerSocketManager*> _server_socket_managers;
	ServerEvent _event;
	ServerLogger& _server_logger;
};

#endif // SERVER_SERVER_HPP
