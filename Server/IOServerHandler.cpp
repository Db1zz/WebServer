#include "IOServerHandler.hpp"

#include "IOServerContext.hpp"
#include "ServerLogger.hpp"
#include "ServerSocket.hpp"
#include "ServerSocketManager.hpp"

IOServerHandler::IOServerHandler(ServerSocket& server_socket, IOServerContext& server_context,
								 ServerLogger* server_logger)
	: _server_socket(server_socket),
	  _server_context(server_context),
	  _server_logger(server_logger),
	  _timeout_timer(NULL) {
}

void IOServerHandler::handle(void* data) {
	try {
		_server_context.server_socket_manager->accept_connection();
	} catch (const std::exception& e) {
		if (_server_logger != NULL) {
			_server_logger->log_error("IOServerHandler::handle",
									  "failed to accept new connection: '" + std::string(e.what()) + "'");
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