#ifndef SERVER_IO_CLIENT_CONTEXT_HPP_
#define SERVER_IO_CLIENT_CONTEXT_HPP_

#include <string>

#include "IIOContext.hpp"
#include "IOEpollContext.hpp"
#include "RequestParser/ServerRequestParser.hpp"
#include "ServerRequest.hpp"

class ServerLogger;
class ClientSocket;
class ServerSocket;
class ServerSocketManager;

class IOClientContext : public IOEpollContext {
   public:
	ClientSocket& client_socket;
	ServerSocket& server_socket;
	ServerSocketManager& server_socket_manager;
	const t_config* server_config;
	ServerLogger* server_logger;
	t_request request;
	std::string buffer;
	pid_t cgi_pid;
	ServerRequestParser parser; // make an interface class bro)

	void reset() {
		request = t_request();
		buffer.clear();
		cgi_pid = -1;
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
		  parser(&request, server_config, server_logger) {}
};

#endif // SERVER_IO_CLIENT_CONTEXT_HPP_
