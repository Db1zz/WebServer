#ifndef SERVER_IO_CLIENT_HANDLER_HPP_
#define SERVER_IO_CLIENT_HANDLER_HPP_

#include "IIOHandler.hpp"
#include "status.hpp"
#include "timer.hpp"

#include <sys/timerfd.h>

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
	bool is_closing() const;
	void set_timeout_timer(ITimeoutTimer* timeout_timer);

   private:
	Status read_and_parse();
	Status handle_default_request(Status status);
	Status handle_cgi_request(Status status);
	Status create_cgi_process();

	ClientSocket& _client_socket;
	IOClientContext& _client_context;
	ServerEvent& _server_event;
	ServerLogger* _server_logger;
	ITimeoutTimer* _timeout_timer;
	bool _is_closing;
};

#endif // SERVER_IO_CLIENT_HANDLER_HPP_