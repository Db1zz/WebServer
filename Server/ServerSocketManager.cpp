#include "ServerSocketManager.hpp"

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "ClientSocket.hpp"
#include "ServerEvent.hpp"
#include "status.hpp"

ServerSocketManager::ServerSocketManager(const std::string& server_socket_host,
										 int server_socket_port, ServerEvent* event_system,
										 const t_config& server_config)
	: _server_socket(server_socket_host, server_socket_port),
	  _event_system(event_system),
	  _server_config(server_config) {
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
	if (!status) {
		return status;
	}
	return register_server_socket_in_event_system();
}

Status ServerSocketManager::stop() {
	Status status;

	destroy_all_clients();
	_server_socket.close_socket();
	return unregister_server_socket_in_event_system();
}

Status ServerSocketManager::accept_connection() {
	Status status;
	ClientSocket* client_socket;

	client_socket = new ClientSocket(&_server_config);

	status = _server_socket.accept_connection(*client_socket);
	if (!status) {
		delete client_socket;
		return status;
	}

	if (fcntl(client_socket->get_fd(), F_SETFL, O_NONBLOCK) < 0) {
		delete client_socket;
		return Status(std::string("ServerSocketManager failed to accept incoming connection: ") + strerror(errno));
	}

	status = register_client_socket_in_event_system(client_socket);
	if (!status) {
		delete client_socket;
		return status;
	}
	_clients.insert(std::make_pair(client_socket->get_fd(), client_socket));

	return Status::OK();
}

Status ServerSocketManager::close_connection_with_client(int client_socket_fd) {
	Status status;
	ClientSocket* client_socket;

	status = get_client_socket(client_socket_fd, &client_socket);
	if (!status) {
		return status;
	}

	unregister_client_socket_in_event_system(client_socket_fd);
	delete client_socket;
	_clients.erase(client_socket_fd);
	return Status::OK();
}

Status ServerSocketManager::get_client_socket(int client_socket_fd, ClientSocket** out) {
	std::map<int, ClientSocket*>::iterator client_it = _clients.find(client_socket_fd);
	if (client_it == _clients.end()) {
		return Status(
			"ServerSocketManager failed to close connection with a client: client not found");
	}
	*out = client_it->second;
	return Status::OK();
}

const ServerSocket* ServerSocketManager::get_server_socket() const {
	return &_server_socket;
}

Status ServerSocketManager::register_client_socket_in_event_system(ClientSocket* client_socket) {
	Status status;

	status = _event_system->add_event(SERVER_EVENT_CLIENT_EVENTS, client_socket);
	if (!status) {
		return Status("ServerSocketManager failed to register client socket in event system: " +
					  status.msg());
	}
	return Status::OK();
}

Status ServerSocketManager::register_server_socket_in_event_system() {
	Status status;

	status = _event_system->add_event(SERVER_EVENT_CLIENT_EVENTS, &_server_socket);
	if (!status) {
		return Status("ServerSocketManager failed to register server socket in event system: " +
					  status.msg());
	}
	return Status::OK();
}

Status ServerSocketManager::unregister_client_socket_in_event_system(int client_socket_fd) {
	Status status;

	status = _event_system->remove_event(client_socket_fd);
	if (!status) {
		return Status("ServerSocketManager failed to unregister client socket from event system: " +
					  status.msg());
	}
	return Status::OK();
}

Status ServerSocketManager::unregister_server_socket_in_event_system() {
	Status status;

	status = _event_system->remove_event(_server_socket.get_fd());
	if (!status) {
		return Status("ServerSocketManager failed to unregister server socket from event system: " +
					  status.msg());
	}
	return Status::OK();
}

void ServerSocketManager::destroy_all_clients() {
	std::map<int, ClientSocket*>::iterator it;

	it = _clients.begin();
	while (it != _clients.end()) {
		if (it->second) {
			delete it->second;
		}
		++it;
	}
	_clients.clear();
}

const t_config& ServerSocketManager::get_server_config() const {
	return _server_config;
}