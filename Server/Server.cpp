#include "Server.hpp"

#include <errno.h>
#include <string.h>

#include <cassert>
#include <csignal>
#include <cstdio>
#include <sstream>

#include "Chunk.hpp"
#include "ClientSocket.hpp"
#include "RequestParser/ServerRequestParser.hpp"
#include "ServerConfig.hpp"
#include "ServerLogger.hpp"
#include "ServerRequest.hpp"
#include "ServerResponse.hpp"
#include "ServerSocket.hpp"
#include "ServerSocketManager.hpp"
#include "Socket.hpp"
#include "status.hpp"

namespace {
volatile std::sig_atomic_t g_signal_status = 0;
}

void sigint_handler(int signal) {
	g_signal_status = signal;
}

Server::Server(const std::vector<t_config>& configs, ServerLogger& server_logger)
	: _configs(configs), _server_logger(server_logger) {
}

Server::~Server() {
	destroy_all_server_socket_managers();
}

Status Server::launch() {
	Status status;
	int amount_of_events = 0;

	status = create_sockets_from_configs(_configs);
	if (!status) {
		_server_logger.log_error("Server::launch()", status.msg());
		return status;
	}

	std::signal(SIGINT, sigint_handler);
	while (g_signal_status != SIGINT) {
		status = _event.wait_event(-1, &amount_of_events);
		if (!status && status.error() != EINTR) {
			throw std::runtime_error("wait_event() failed in Server::Launch(): " + status.msg());
		}

		if (amount_of_events > 0) {
			status = handle_event(amount_of_events);
			if (!status) {
				throw std::runtime_error("handle_event() failed in Server::Launch(): " +
										 status.msg());
			}
		}
	}
	std::cout << "[Server] shutdown..." << std::endl;
	return Status::OK();
}

bool Server::is_a_new_connection(const epoll_event& event) {
	Socket* event_socket = static_cast<Socket*>(event.data.ptr);
	return find_server_socket_manager(event_socket->get_fd()) != NULL;
}

Status Server::handle_new_connection_event(const epoll_event& connection_event) {
	Status status;
	Socket* socket;

	socket = static_cast<Socket*>(connection_event.data.ptr);
	ServerSocketManager* manager = find_server_socket_manager(socket->get_fd());
	if (!manager) {
		return Status(
			"Server failed to handle new connection event: ServerSocketManager was not found");
	}

	status = manager->accept_connection();
	if (!status) {
		return status;
	}

	return Status::OK();
}

Status Server::handle_request_event(const epoll_event& request_event) {
	Status status;
	ClientSocket* client_socket = NULL;
	ServerSocketManager* manager = NULL;

	client_socket = static_cast<ClientSocket*>(request_event.data.ptr);
	if (request_event.events & (EPOLLERR | EPOLLRDHUP)) {
		manager = find_server_socket_manager(client_socket->get_server_fd());
		if (!manager) {
			return Status("Server cannot find a server to close connection with");
		}
		manager->close_connection_with_client(client_socket->get_fd());
		std::cout << "destroy client\n";
	} else if (request_event.events & EPOLLIN) {
		status = request_handler(client_socket);

		if (status.error() != DataIsNotReady) {
			status = response_handler(client_socket);
			if (!status) {
				return Status("response_handler() failed in Server::handle_event(): " +
							  status.msg());
			}

			ClientConnectionContext* connection_context = client_socket->get_connection_context();
			if (connection_context->request.is_request_ready()) {
				manager = find_server_socket_manager(client_socket->get_server_fd());
				_server_logger.log_access(
					*client_socket->get_host(), connection_context->request.method,
					connection_context->request.uri_path, manager->get_server_socket()->get_port());
				client_socket->reset_connection_context();
			}
		}
	}
	return Status::OK();
}

Status Server::handle_event(int amount_of_events) {
	Status status;

	for (int i = 0; i < amount_of_events; ++i) {
		const epoll_event& event = *_event[i];
		if (is_a_new_connection(event)) {
			status = handle_new_connection_event(event);
			if (!status) {
				_server_logger.log_error("Server::handle_event()", status.msg());
			}
			continue;
		}
		status = handle_request_event(event);
		if (!status) {
			_server_logger.log_error("Server::handle_event()", status.msg());
			continue;
		}
	}
	return Status::OK();
}

Status Server::read_data(ClientSocket* client_socket, std::string& buff, int& rd_bytes) {
	const ssize_t read_buff_size = READ_BUFFER_SIZE;
	char read_buff[read_buff_size];

	rd_bytes = read(client_socket->get_fd(), read_buff, read_buff_size);
	if (rd_bytes == 0) {
		return Status::InternalServerError();
	}
	if (rd_bytes < 0) {
		return Status::InternalServerError();
	}
	buff.append(read_buff, rd_bytes);
	return Status::OK();
}

/*
	New Request -> Request Handler -> Read Request Chunk ->
	Get Request Data -> Pass Chunk To Request Parser

	The instance of the ServerRequesParser will have a cache
	that stores all relaible to request data.
*/
Status Server::request_handler(ClientSocket* client_socket) {
	Status status;
	ClientConnectionContext* connection_context = client_socket->get_connection_context();
	std::string buffer;
	int rd_bytes;

	status = read_data(client_socket, buffer, rd_bytes);
	// std::cout << GREEN300 << "REQUEST:" << buffer << RESET << std::endl;
	if (!status) {
		return Status("Server::read_data " + status.msg());
	}

	status = connection_context->parser.feed(buffer);
	if (!status) {
		return status;
	}

	return Status::OK();
}

Status Server::response_handler(ClientSocket* client_socket) {
	ServerResponse resp(client_socket, _configs[0]);
	resp.generate_response();

	if (resp.status.code() == 100) {
		return Status();
	}
	if (resp.needs_streaming()) {
		std::string headers = resp.get_response();
		if (write(client_socket->get_fd(), headers.c_str(), headers.size()) < 0)
			return Status("failed to send response headers to client");
		Status stream_status = Chunk::stream_file_chunked(
			resp.get_stream_file_path(), client_socket->get_fd(), resp.get_stream_location());
		if (!stream_status.is_ok()) return stream_status;
	} else {
		std::string res = resp.get_response();
		if (write(client_socket->get_fd(), res.c_str(), res.size()) < 0)
			return Status("failed to send response to client");
	}
	if (resp.status.code() == BadRequest || resp.status.code() == Conflict) {
		ServerSocketManager* manager = find_server_socket_manager(client_socket->get_server_fd());
		manager->close_connection_with_client(client_socket->get_fd());
		return Status::CloseConnection();
		//! IN THIS FUNCTION WE NEED TO DO MODIFICATIONS
		// TODO:
		// 1. maybe add a separate function for handling streaming from server
		// 2. add all of the codes (maybe convert it to string and check fisrt char to see if its an
		// error code ot not hmmm, lets discuss)
	}
	return Status();
}

Status Server::create_server_socket_manager(const std::string& host, int port,
											const t_config& server_config) {
	Status status;
	ServerSocketManager* manager;

	manager = new ServerSocketManager(host, port, &_event, server_config);
	status = manager->start();
	if (!status) {
		delete manager;
		return status;
	}
	_server_socket_managers.insert(std::make_pair(manager->get_server_socket()->get_fd(), manager));
	return Status::OK();
}

Status Server::create_sockets_from_config(const t_config& server_config) {
	const size_t amount_of_addresses = server_config.listen.size();
	Status status;

	for (size_t i = 0; i < amount_of_addresses; ++i) {
		const std::string& host = server_config.listen[i].host;
		int port = server_config.listen[i].port;

		status = create_server_socket_manager(host, port, server_config);
		if (!status) {
			return status;
		}
	}
	return Status::OK();
}

Status Server::create_sockets_from_configs(const std::vector<t_config>& configs) {
	Status status;

	for (size_t i = 0; i < configs.size(); ++i) {
		const t_config& config = configs[i];

		status = create_sockets_from_config(config);
		if (!status) {
			return status;
		}
	}
	return Status::OK();
}

ServerSocketManager* Server::find_server_socket_manager(int server_socket_fd) {
	std::map<int, ServerSocketManager*>::iterator it =
		_server_socket_managers.find(server_socket_fd);
	if (it == _server_socket_managers.end()) {
		return NULL;
	}
	return it->second;
}

void Server::destroy_all_server_socket_managers() {
	std::map<int, ServerSocketManager*>::iterator it;

	it = _server_socket_managers.begin();
	while (it != _server_socket_managers.end()) {
		if (it->second) {
			delete it->second;
		}
		++it;
	}
	_server_socket_managers.clear();
}

void Server::print_debug_addr(const std::string& address, int port) {
	std::cout << GREEN400 << "Listening at: " << address << ":" << port << RESET << std::endl;
}
