#ifndef SERVER_IO_CLIENT_CONTEXT_HPP_
#define SERVER_IO_CLIENT_CONTEXT_HPP_

#include <string>

#include "RequestParser/ServerRequestParser.hpp"
#include "ServerRequest.hpp"

class ServerLogger;
class ClientSocket;
class ServerSocket;
class ServerSocketManager;
class IIOContext;

class IOClientContext : public IIOContext {
   public:
	ClientSocket& client_socket;
	ServerSocket& server_socket;
	ServerSocketManager& server_socket_manager;
	const t_config* server_config;
	ServerLogger* server_logger;
	t_request request;
	std::string buffer;
	pid_t cgi_pid;
	int cgi_fd;
	ServerRequestParser parser;

	void reset() {
		request = t_request();
		buffer.clear();
		cgi_pid = -1;
		cgi_fd = -1;
		parser = ServerRequestParser(&request, server_config, server_logger);
	}

	IOClientContext(ClientSocket& client_socket, ServerSocket& server_socket,
					ServerSocketManager& server_socket_manager, const t_config* server_config,
					ServerLogger* server_logger)
		: client_socket(client_socket),
		  server_socket(server_socket),
		  server_socket_manager(server_socket_manager),
		  server_config(server_config),
		  server_logger(server_logger),
		  cgi_pid(-1),
		  cgi_fd(-1),
		  parser(&request, server_config, server_logger) {}
};

#endif // SERVER_IO_CLIENT_CONTEXT_HPP_
