#include "ServerSocketManager.hpp"

#include <errno.h>
#include <signal.h>
#include <string.h>

#include "ClientEventContext.hpp"
#include "ClientSocket.hpp"
#include "IOClientContext.hpp"
#include "IOClientHandler.hpp"
#include "ServerEvent.hpp"
#include "ServerLogger.hpp"

ServerSocketManager::ServerSocketManager(const std::string& server_socket_host,
										 int server_socket_port, ServerEvent* event_system,
										 const t_config& server_config, ServerLogger* server_logger)
	: _server_socket(server_socket_host, server_socket_port),
	  _event_system(event_system),
	  _server_config(server_config),
	  _server_logger(server_logger) {
}

ServerSocketManager::~ServerSocketManager() {
	if (_server_socket.is_connected()) {
		_server_socket.close_socket();
	}
}

void ServerSocketManager::start() {
	_server_socket.open_socket();
}

void ServerSocketManager::stop() {
	_server_socket.close_socket();
}

void ServerSocketManager::accept_connection() {
	ClientSocket* client_socket = new ClientSocket(&_server_config);
	IOClientContext* io_client_context =
		new IOClientContext(*client_socket, _server_socket, *this, &_server_config, _server_logger);
	IOClientHandler* io_client_handler =
		new IOClientHandler(*client_socket, *io_client_context, *_event_system, _server_logger);
	ClientEventContext* client_event_context = new ClientEventContext();
	client_event_context->take_data_ownership(io_client_handler, io_client_context, client_socket, NULL);

	try {
		_server_socket.accept_connection(*client_socket);
		std::cout << "Server accepted new connection with id: " << client_socket->get_fd() << std::endl;
		client_socket->set_nonblock();
		_event_system->register_event(SERVER_EVENT_CLIENT_EVENTS, client_socket->get_fd(),
										   client_event_context);
	} catch(const std::exception& e) {
		delete client_socket;
		if (_server_logger != NULL) {
			_server_logger->log_error("ServerSocketManager::accept_connection()", strerror(errno));
		}
		throw;
	}
}

ServerSocket* ServerSocketManager::get_server_socket() {
	return &_server_socket;
}

const t_config& ServerSocketManager::get_server_config() const {
	return _server_config;
}
