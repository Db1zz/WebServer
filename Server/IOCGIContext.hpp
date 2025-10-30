#ifndef SERVER_IO_CGI_CONTEXT_HPP_
#define SERVER_IO_CGI_CONTEXT_HPP_

#include <string>

#include "IIOContext.hpp"
#include "ServerRequest.hpp"

class ServerLogger;
class CGIFileDescriptor;
class CGIResponseParser;

class IOCGIContext : public IIOContext {
   public:
   	CGIFileDescriptor& cgi_fd;
   	const t_config* server_config;
   	ServerLogger* server_logger;
	CGIResponseParser* cgi_parser;
	t_request request;
	std::string buffer;
	pid_t cgi_pid;

	IOCGIContext(CGIFileDescriptor& cgi_fd, const t_config* server_config, ServerLogger* server_logger) 
		: cgi_fd(cgi_fd),
		  server_config(server_config),
		  server_logger(server_logger),
		  cgi_pid(-1) 
		  {
			request.is_cgi = true;
		  }

	void reset() {}
};

#endif // SERVER_IO_CGI_CONTEXT_HPP_