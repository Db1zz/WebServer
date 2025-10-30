#include "ServerSocketManager.hpp"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#include "ServerLogger.hpp"
#include "ClientSocket.hpp"
#include "ServerEvent.hpp"
#include "status.hpp"
#include "IOClientHandler.hpp"
#include "IOClientContext.hpp"

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
	destroy_all_clients();
}

Status ServerSocketManager::start() {
	Status status;

	status = _server_socket.open_socket();
	return status;
}

Status ServerSocketManager::stop() {
	Status status;

	destroy_all_clients();
	_server_socket.close_socket();
	return status;
}

Status ServerSocketManager::accept_connection() {
	Status status;
	ClientSocket* client_socket = new ClientSocket(&_server_config);
	IOClientContext* io_client_context = new IOClientContext(*client_socket, _server_socket, *this, &_server_config, _server_logger);
	IOClientHandler* io_client_handler = new IOClientHandler(*client_socket, *io_client_context, *_event_system, _server_logger);
	EventContext *event_context = new EventContext(io_client_context, io_client_handler);

	status = _server_socket.accept_connection(*client_socket);
	if (!status) {
		delete client_socket;
		return status;
	}

	if (fcntl(client_socket->get_fd(), F_SETFL, O_NONBLOCK) < 0) {
		delete client_socket;
		return Status(std::string("ServerSocketManager failed to accept incoming connection: ") +
					  strerror(errno));
	}

	status = _event_system->add_event(SERVER_EVENT_CLIENT_EVENTS, client_socket->get_fd(), *event_context);
	if (!status) {
		delete client_socket;
		return status;
	}
	_clients_contexts.insert(std::make_pair(client_socket->get_fd(), event_context));

	return Status::OK();
}

Status ServerSocketManager::close_connection_with_client(int client_socket_fd) {
	Status status;
	EventContext* client_event_context;

	status = get_client_event_context(client_socket_fd, &client_event_context);
	if (!status) {
		_server_logger->log_error("ServerSocketManager::close_connection_with_client", "failed to obtain client event context: " + status.msg());
		return status;
	}

	IOClientContext* io_client_context = static_cast<IOClientContext*>(client_event_context->context);
	if (io_client_context->cgi_pid >= 0) {
		kill(io_client_context->cgi_pid, SIGKILL);
		std::cout << "Killing CGI process with pid " << io_client_context->cgi_pid << std::endl;
		io_client_context->cgi_pid = -1;
	}
	
	delete &io_client_context->client_socket;
	delete client_event_context->context;
	delete client_event_context->handler;
	delete client_event_context;

	_clients_contexts.erase(client_socket_fd);
	_event_system->remove_event(client_socket_fd);
	return Status::OK();
}

Status ServerSocketManager::get_client_event_context(int client_socket_fd, EventContext** out) {
	std::map<int, EventContext*>::iterator client_context_it = _clients_contexts.find(client_socket_fd);
	if (client_context_it == _clients_contexts.end()) {
		return Status("client event context not found");
	}

	*out = client_context_it->second;
	return Status::OK();
}

ServerSocket* ServerSocketManager::get_server_socket() {
	return &_server_socket;
}

void ServerSocketManager::destroy_all_clients() {
	std::map<int, EventContext*>::iterator client_context_it;

	client_context_it = _clients_contexts.begin();
	while (client_context_it != _clients_contexts.end()) {
		std::map<int, EventContext*>::iterator curr_it = client_context_it;
		++client_context_it;
		IOClientContext* io_client_context = static_cast<IOClientContext*>(curr_it->second->context);
		close_connection_with_client(io_client_context->client_socket.get_fd());
	}
	_clients_contexts.clear();
}

const t_config& ServerSocketManager::get_server_config() const {
	return _server_config;
}

const std::map<int, EventContext*>& ServerSocketManager::get_connected_clients() const {
	return _clients_contexts;
}