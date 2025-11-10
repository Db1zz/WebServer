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
#include "status.hpp"

ServerSocketManager::ServerSocketManager(const std::string& server_socket_host,
										 int server_socket_port, ServerEvent* event_system,
										 const t_config& server_config, ServerLogger* server_logger)
	: _server_socket(server_socket_host, server_socket_port),
	  _event_system(event_system),
	_server_config(server_config),
	_server_logger(server_logger),
	_session_store() {
}

ServerSocketManager::~ServerSocketManager() {
	if (_server_socket.is_connected()) {
		_server_socket.close_socket();
	}
}

Status ServerSocketManager::start() {
	Status status;

	status = _server_socket.open_socket();
	return status;
}

Status ServerSocketManager::stop() {
	Status status;

	_server_socket.close_socket();
	return status;
}

Status ServerSocketManager::accept_connection() {
	Status status;
	ClientSocket* client_socket = new ClientSocket(&_server_config);
	IOClientContext* io_client_context =
		new IOClientContext(*client_socket, _server_socket, *this, &_server_config, _server_logger);
	IOClientHandler* io_client_handler =
		new IOClientHandler(*client_socket, *io_client_context, *_event_system, _server_logger);
	ClientEventContext* client_event_context = new ClientEventContext();
	client_event_context->take_data_ownership(io_client_handler, io_client_context, client_socket, NULL);

	std::cout << "Server accepted new connection with id: " << client_socket->get_fd() << std::endl;
	status = _server_socket.accept_connection(*client_socket);
	if (!status) {
		delete client_socket;
		return status;
	}

	if (client_socket->set_nonblock() == false) {
		delete client_socket;
		return Status(std::string("ServerSocketManager failed to accept incoming connection: ") +
					  strerror(errno));
	}

	status = _event_system->register_event(SERVER_EVENT_CLIENT_EVENTS, client_socket->get_fd(),
										   client_event_context);
	if (!status) {
		delete client_socket;
		return status;
	}

	return Status::OK();
}

// Status ServerSocketManager::close_connection_with_client(int client_socket_fd) {
// 	Status status;
// 	EventContext* client_event_context;

// 	IOClientContext* io_client_context =
// static_cast<IOClientContext*>(client_event_context->context); 	if (io_client_context->cgi_pid >=
// 0) { 		kill(io_client_context->cgi_pid, SIGKILL); 		std::cout << "Killing CGI process with pid " <<
// io_client_context->cgi_pid << std::endl; 		io_client_context->cgi_pid = -1;
// 	}

// 	_event_system->remove_event(client_socket_fd);
// 	return Status::OK();
// }

ServerSocket* ServerSocketManager::get_server_socket() {
	return &_server_socket;
}

const t_config& ServerSocketManager::get_server_config() const {
	return _server_config;
}

SessionStore& ServerSocketManager::get_session_store() {
	return _session_store;
}
