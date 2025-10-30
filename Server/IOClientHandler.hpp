#ifndef SERVER_IO_CLIENT_HANDLER_HPP_
#define SERVER_IO_CLIENT_HANDLER_HPP_

#include "IIOHandler.hpp"
#include "status.hpp"

class ServerEvent;
class ServerLogger;
class ClientSocket;
class IOClientContext;
class EventContext;

class IOClientHandler : public IIOHandler {
   public:
	IOClientHandler(ClientSocket& client_socket, IOClientContext& client_context,
					ServerEvent& server_event, ServerLogger* server_logger);
	~IOClientHandler();
	Status handle(void* data);

   private:
	Status close_connection();
	Status read_and_parse();
	Status handle_default_request(Status status);
	Status handle_cgi_request(Status status);
	Status create_cgi_process();

	ClientSocket& _client_socket;
	IOClientContext& _client_context;
	ServerEvent& _server_event;
	ServerLogger* _server_logger;
	EventContext* _cgi_event_context;
};

#endif // SERVER_IO_CLIENT_HANDLER_HPP_