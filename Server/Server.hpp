#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include <map>
#include <string>
#include <vector>

#include "ServerConfig.hpp"
#include "ServerEvent.hpp"
#include "ServerRequest.hpp"

#define WS_PROTOCOL "HTTP/1.1"

class ServerLogger;
class IEventContext;

class Server {
   public:
	Server(const std::vector<t_config>& configs, ServerLogger& server_logger);
	~Server();
	void launch();

   private:
	void handle_epoll_event(int amount_of_events);
	void create_server_socket(const std::string& host, int port, const t_config& server_config);
	void create_sockets_from_config(const t_config& server_config);
	void create_sockets_from_configs(const std::vector<t_config>& configs);
	void print_debug_addr(const std::string& address, int port);
	// void destroy_server_event_contexts();


	std::vector<t_config> _configs;
	ServerEvent _event;
	ServerLogger& _server_logger;
	std::vector<IEventContext*> _server_event_contexts;
};

#endif // SERVER_SERVER_HPP
