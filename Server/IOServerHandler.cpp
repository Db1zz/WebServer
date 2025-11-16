#include "IOServerHandler.hpp"

#include <cstdlib>

#include "ClientEventContext.hpp"
#include "ClientSocket.hpp"
#include "IOClientContext.hpp"
#include "IOClientHandler.hpp"
#include "IOServerContext.hpp"
#include "ServerEvent.hpp"
#include "ServerLogger.hpp"
#include "ServerSocket.hpp"

#include "DolbayobPTR.hpp"

IOServerHandler::IOServerHandler(ServerSocket& server_socket, IOServerContext& server_context,
								 ServerEvent& server_event, ServerLogger* server_logger)
	: _server_socket(server_socket),
	  _server_context(server_context),
	  _server_event(server_event),
	  _server_logger(server_logger),
	  _timeout_timer(NULL) {
}

void IOServerHandler::handle(void* data) {
	(void)data;
	ClientSocket* client_socket = new ClientSocket(&_server_socket.get_server_config());
	IOClientContext* io_client_context =
		new IOClientContext(*client_socket, _server_socket, _server_logger);
	IOClientHandler* io_client_handler =
		new IOClientHandler(*client_socket, *io_client_context, _server_event, _server_logger);
	DolbayobPTR<IEventContext> client_event_context(new ClientEventContext());
	client_event_context.get()->take_data_ownership(io_client_handler, io_client_context, client_socket,
											  NULL);

	try {
		_server_socket.accept_connection(*client_socket);
		std::cout << "Server accepted new connection with id: " << client_socket->get_fd()
				  << std::endl;
		client_socket->set_nonblock();
		if (!_server_event.register_event(SERVER_EVENT_CLIENT_EVENTS, client_socket->get_fd(),
										  client_event_context)) {
			std::cout << "TODO vpadlu2\n";
			std::exit(127);
		}
	} catch (const std::exception& e) {
		if (_server_logger != NULL) {
			_server_logger->log_error("IOServerHandler::handle()", strerror(errno));
		}
		throw;
	}
}

void IOServerHandler::set_timeout_timer(ITimeoutTimer* timeout_timer) {
	_timeout_timer = timeout_timer;
}


bool IOServerHandler::is_closing() const {
	return false;
}

void IOServerHandler::set_is_closing() {
	return;
}