#ifndef SERVER_IO_CLIENT_CONTEXT_HPP_
#define SERVER_IO_CLIENT_CONTEXT_HPP_

#include <string>

#include "RequestParser/ServerRequestParser.hpp"
#include "ServerRequest.hpp"
#include "ServerSocket.hpp"
#include "IEventContext.hpp"
#include "DolbayobPTR.hpp"

class ServerLogger;
class ClientSocket;
class IIOContext;

class IOClientContext : public IIOContext {
   public:
	ClientSocket& client_socket;
	ServerSocket& server_socket;
	ServerLogger* server_logger;
	t_request request;
	std::string buffer;
	ServerRequestParser parser;

	DolbayobPTR<IEventContext> cgi_event_context;
	bool is_cgi_request_finished;
	bool cgi_started;
	int cgi_fd;
	std::string cgi_buffer;

	void reset() {
		request = t_request();
		buffer.clear();
		parser.reset();

		cgi_event_context.unlink_ptr();
		is_cgi_request_finished = false;
		cgi_started = false;
		cgi_fd = -1;
		cgi_buffer.clear();
	}

	IOClientContext(ClientSocket& client_socket, ServerSocket& server_socket,
					ServerLogger* server_logger)
		: client_socket(client_socket),
		  server_socket(server_socket),
		  server_logger(server_logger),
		  parser(&server_socket.get_server_config(), server_logger),
		  cgi_event_context(NULL),
		  is_cgi_request_finished(false),
		  cgi_started(false),
		  cgi_fd(-1) {}
};

#endif // SERVER_IO_CLIENT_CONTEXT_HPP_
