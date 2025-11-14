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

	void handle(void* data);
	bool is_closing() const;
	void set_is_closing();
	void set_timeout_timer(ITimeoutTimer* timeout_timer);

   private:
	void handle_timeout();
	bool handle_default();

	Status _status;
	CGIFileDescriptor& _cgi_fd;
	IOCGIContext& _io_cgi_context;
	IOClientContext& _io_client_context;
	const t_config* _server_config;
	ServerLogger* _server_logger;
	ITimeoutTimer* _timeout_timer;
	bool _is_closing;
	bool _child_exited;
	std::string _buffer;
};

#endif // SERVER_IO_CGI_HANDLER_HPP_