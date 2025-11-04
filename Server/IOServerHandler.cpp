#include "IOServerHandler.hpp"

#include "ServerSocket.hpp"
#include "IOServerContext.hpp"
#include "ServerLogger.hpp"
#include "ServerSocketManager.hpp"

IOServerHandler::IOServerHandler(ServerSocket& server_socket, IOServerContext& server_context,
								 ServerLogger* server_logger)
	: _server_socket(server_socket),
	  _server_context(server_context),
	  _server_logger(server_logger) {
}

Status IOServerHandler::handle(void* data) {
	Status status;

	status = _server_context.server_socket_manager->accept_connection();
	if (!status && _server_logger != NULL) {
		_server_logger->log_error("IOServerHandler::handle", "failed to accept new connection: '" + status.msg() + "'");
	}
	return status;
}

bool IOServerHandler::is_closing() const {
	return false;
}