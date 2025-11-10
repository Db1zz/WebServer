#ifndef SERVER_SERVER_SOCKET_MANAGER_HPP_
#define SERVER_SERVER_SOCKET_MANAGER_HPP_

#include <map>

#include "ServerConfig.hpp"
#include "ServerSocket.hpp"
#include "ServerEvent.hpp"
#include "SessionStore.hpp"

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
	// Status close_connection_with_client(int client_socket_fd);
	ServerSocket* get_server_socket();
	const t_config& get_server_config() const;
	SessionStore& get_session_store();

   private:

	ServerSocket _server_socket;
	ServerEvent* _event_system;
	t_config _server_config;
	ServerLogger* _server_logger;
	SessionStore _session_store;
};

#endif // SERVER_SERVER_SOCKET_MANAGER_HPP_