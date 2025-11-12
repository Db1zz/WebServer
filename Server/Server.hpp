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

	bool check_if_can_destroy_event(int events, IEventContext& event_context,
									std::map<int, IEventContext*>& events_to_destroy);
	bool is_object_expired(IEventContext& event_context);
	void destroy_events(std::map<int, IEventContext*>& events);

	std::vector<t_config> _configs;
	ServerEvent _event;
	ServerLogger& _server_logger;
	SessionStore _session_store;
};

#endif // SERVER_SERVER_HPP
