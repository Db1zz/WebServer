#ifndef SERVER_IO_SERVER_HANDLER_HPP_
#define SERVER_IO_SERVER_HANDLER_HPP_

#include "IIOHandler.hpp"
#include "status.hpp"

class IOServerContext;
class ServerLogger;
class ServerSocket;

class IOServerHandler : public IIOHandler {
   public:
	IOServerHandler(ServerSocket& server_socket, IOServerContext& server_context,
					ServerLogger* server_logger);

	Status handle(void* data);

   private:
	ServerSocket& _server_socket;
	IOServerContext& _server_context;
	ServerLogger* _server_logger;
};

#endif // SERVER_IO_SERVER_HANDLER_HPP_