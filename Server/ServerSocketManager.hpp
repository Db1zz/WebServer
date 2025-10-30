#ifndef SERVER_SERVER_SOCKET_MANAGER_HPP_
#define SERVER_SERVER_SOCKET_MANAGER_HPP_

#include <map>

#include "ServerConfig.hpp"
#include "ServerSocket.hpp"
#include "ServerEvent.hpp"

class ServerLogger;
class ClientSocket;
class Status;

class ServerSocketManager {
   public:
	explicit ServerSocketManager(const std::string& server_socket_host, int server_socket_port,
								 ServerEvent* event_system, const t_config& server_config,
								 ServerLogger* server_logger);
	~ServerSocketManager();

	Status start();
	Status stop();
	Status accept_connection();
	Status close_connection_with_client(int client_socket_fd);
	Status get_client_event_context(int client_socket_fd, EventContext** out);
	ServerSocket* get_server_socket();
	const t_config& get_server_config() const;
	const std::map<int, EventContext*>& get_connected_clients() const;

   private:
	void destroy_all_clients();

	std::map<int, EventContext*> _clients_contexts; // pair<int fd, ClientSocket*>
	ServerSocket _server_socket;
	ServerEvent* _event_system;
	t_config _server_config;
	ServerLogger* _server_logger;
};

#endif // SERVER_SERVER_SOCKET_MANAGER_HPP_