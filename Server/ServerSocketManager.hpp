#ifndef SERVER_SERVER_SOCKET_MANAGER_HPP_
#define SERVER_SERVER_SOCKET_MANAGER_HPP_

#include <map>

#include "ServerSocket.hpp"

class ServerEvent;
class ClientSocket;
class ServerSocket;
class Status;

class ServerSocketManager {
   public:
	explicit ServerSocketManager(const std::string& server_socket_host,
								 int server_socket_port, ServerEvent* event_system);
	~ServerSocketManager();

	Status start();
	Status stop();
	Status accept_connection();
	Status close_connection_with_client(int client_socket_fd);
	Status get_client_socket(int client_socket_fd, ClientSocket** out);
	const ServerSocket* get_server_socket();

   private:
	Status register_client_socket_in_event_system(ClientSocket* client_socket);
	Status register_server_socket_in_event_system();
	Status unregister_client_socket_in_event_system(int client_socket_fd);
	Status unregister_server_socket_in_event_system();
	void destroy_all_clients();

	std::map<int, ClientSocket*> _clients; // pair<int fd, ClientSocket*>
	ServerSocket _server_socket;
	ServerEvent* _event_system;
};

#endif // SERVER_SERVER_SOCKET_MANAGER_HPP_