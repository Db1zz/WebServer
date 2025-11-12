#ifndef SERVER_IO_SERVER_HANDLER_HPP_
#define SERVER_IO_SERVER_HANDLER_HPP_

#include "IIOHandler.hpp"
#include "status.hpp"

class IOServerContext;
class ServerLogger;
class ServerSocket;
class ServerEvent;

class IOServerHandler : public IIOHandler {
   public:
	IOServerHandler(ServerSocket& server_socket, IOServerContext& server_context,
					ServerEvent& server_event, ServerLogger* server_logger);

	void handle(void* data);
	bool is_closing() const;
	void set_timeout_timer(ITimeoutTimer* timeout_timer);

   private:
	ServerSocket& _server_socket;
	IOServerContext& _server_context;
	ServerEvent& _server_event;
	ServerLogger* _server_logger;
	ITimeoutTimer* _timeout_timer;
};

#endif // SERVER_IO_SERVER_HANDLER_HPP_