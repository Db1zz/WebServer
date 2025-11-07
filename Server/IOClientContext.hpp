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
	ServerRequestParser parser;

	bool is_cgi_request_finished;
	bool cgi_started;
	int cgi_fd;

	void reset() {
		request = t_request();
		buffer.clear();
		parser = ServerRequestParser(&request, server_config, server_logger);
		is_cgi_request_finished = false;
		cgi_started = false;
		cgi_fd = -1;
	}

	IOClientContext(ClientSocket& client_socket, ServerSocket& server_socket,
					ServerSocketManager& server_socket_manager, const t_config* server_config,
					ServerLogger* server_logger)
		: client_socket(client_socket),
		  server_socket(server_socket),
		  server_socket_manager(server_socket_manager),
		  server_config(server_config),
		  server_logger(server_logger),
		  parser(&request, server_config, server_logger),
		  is_cgi_request_finished(false),
		  cgi_started(false),
		  cgi_fd(-1) {}
};

#endif // SERVER_IO_CLIENT_CONTEXT_HPP_
