#ifndef SERVER_IO_CGI_CONTEXT_HPP_
#define SERVER_IO_CGI_CONTEXT_HPP_

#include <string>

#include "IIOContext.hpp"
#include "ServerRequest.hpp"
#include "CGIResponseParser.hpp"

class ServerLogger;
class CGIFileDescriptor;

class IOCGIContext : public IIOContext {
   public:
   	CGIFileDescriptor& cgi_fd;
   	const t_config* server_config;
   	ServerLogger* server_logger;
	CGIResponseParser cgi_parser;
	t_request request;
	std::string buffer;
	pid_t cgi_pid;
	bool is_finished;

	IOCGIContext(CGIFileDescriptor& cgi_fd, pid_t cgi_pid, const t_config* server_config, ServerLogger* server_logger) 
		: cgi_fd(cgi_fd),
		  server_config(server_config),
		  server_logger(server_logger),
		  cgi_parser(&request, server_logger),
		  cgi_pid(cgi_pid),
		  is_finished(false)
		  {
			request.is_cgi = true;
		  }

	void reset() {
		// NOTE: The cgi context should be non-resetable(can be used only once)

        // request = t_request();
        // buffer.clear();
        // cgi_pid = -1;
		// cgi_parser = CGIResponseParser(&request, server_logger);
		// is_finished = false;
	}
};

#endif // SERVER_IO_CGI_CONTEXT_HPP_