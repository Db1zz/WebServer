#ifndef SERVER_IO_CGI_HANDLER_HPP_
#define SERVER_IO_CGI_HANDLER_HPP_

#include "IIOContext.hpp"
#include "IIOHandler.hpp"
#include "ServerLogger.hpp"
#include "ServerConfig.hpp"
#include "status.hpp"

class CGIFileDescriptor;
class IOCGIContext;
class IOClientContext;

class IOCGIHandler : public IIOHandler {
   public:
	IOCGIHandler(CGIFileDescriptor& cgi_fd, IOCGIContext& io_cgi_context, IOClientContext& io_client_context, const t_config* server_config, ServerLogger* server_logger);
	~IOCGIHandler();
	Status handle(void* data);

   private:
	CGIFileDescriptor& _cgi_fd;
	IOCGIContext& _io_cgi_context;
	IOClientContext& _io_client_context;
	const t_config* _server_config;
	ServerLogger* _server_logger;
};

#endif // SERVER_IO_CGI_HANDLER_HPP_