#ifndef SERVER_SERVER_SOCKET_HPP
#define SERVER_SERVER_SOCKET_HPP

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#include <iostream>
#include <map>

#include "Socket.hpp"
#include "ServerEvent.hpp"
#include "ServerConfig.hpp"

class ClientSocket;
class ServerLogger;
class ServerSocket : public Socket {
   public:
	ServerSocket(const std::string& host, int port, const t_config* server_config, ServerLogger* server_logger);
	~ServerSocket();

	void start();
	void stop();

	void accept_connection(ClientSocket& empty_client_socket);
	const t_config& get_server_config() const;

   private:
   	void open_socket();
	void create_server_socket();

	const t_config* _server_config;
	ServerLogger* _server_logger;
};

#endif // SERVER_SERVER_SOCKET_HPP
