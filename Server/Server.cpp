#include "Server.hpp"

#include <errno.h>
#include <string.h>

#include <csignal>
#include <cstdio>
#include <sstream>

#include "ClientSocket.hpp"
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
		if (!status && status.code() != EINTR) {
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
	return status;
}

bool Server::is_a_new_connection(const epoll_event& event) {
	Socket* event_socket = static_cast<Socket*>(event.data.ptr);
	std::map<int, ServerSocketManager*>::iterator search;

	return find_server_socket_manager(event_socket->get_fd(), search);
}

Status Server::handle_new_connection_event(const epoll_event& connection_event) {
	Status status;
	Socket* socket;
	std::map<int, ServerSocketManager*>::iterator search;

	socket = static_cast<Socket*>(connection_event.data.ptr);
	if (!find_server_socket_manager(socket->get_fd(), search)) {
		return Status(
			"Server failed to handle new connection event: ServerSocketManager was not found");
	}

	status = search->second->accept_connection();
	if (!status) {
		return status;
	}

	return Status();
}

Status Server::handle_request_event(const epoll_event& request_event) {
	Status status;
	ClientSocket* client_socket;
	std::map<int, ServerSocketManager*>::iterator search;

	client_socket = static_cast<ClientSocket*>(request_event.data.ptr);
	if (request_event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
		if (!find_server_socket_manager(client_socket->get_server_fd(), search)) {
			return Status("Server cannot find a server to close connection with");
		}
		search->second->close_connection_with_client(client_socket->get_fd());
	} else if (request_event.events & EPOLLIN) {
		t_request request;
		status = request_handler(client_socket, request);
		if (!status) {
			return Status("request_handler() failed in Server::handle_event(): " + status.msg());
		}
		status = response_handler(client_socket, request);
		if (!status) {
			return Status("response_handler() failed in Server::handle_event(): " + status.msg());
		}
		_server_logger.log_access(*client_socket->get_host(), request.method, request.uri_path,
								  client_socket->get_server_fd());
	}
	return Status();
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
	return Status();
}

t_request Server::request_parser(std::string request) {
	t_request requestStruct;
	requestStruct.mime_type = ""; // if there no mime found -> empty string
	std::stringstream iss(request);
	std::string extract;
	iss >> extract;
	requestStruct.method = extract;
	iss >> extract;
	requestStruct.uri_path = extract;
	if (extract.find('.') != std::string::npos)
		requestStruct.mime_type = extract.substr(extract.find('.'));
	iss >> extract; // we ignore HTTP/1.1 for now
	while (std::getline(iss, extract) || extract != "\r") {
		if (extract.empty() || extract == "\r\n") break;
		if (extract.find("Host: ", 0) != std::string::npos)
			requestStruct.host = extract.substr(6);
		else if (extract.find("User-Agent: ", 0) != std::string::npos)
			requestStruct.user_agent = extract.substr(12);
		else if (extract.find("Accept: ", 0) != std::string::npos)
			requestStruct.accept = extract.substr(8);
		else if (extract.find("Accept-Language: ", 0) != std::string::npos)
			requestStruct.language = extract.substr(17);
		else if (extract.find("Connection: ", 0) != std::string::npos)
			requestStruct.connection = extract.substr(12);
	}
	return requestStruct;
}

Status Server::read_request(const ClientSocket* client_socket, std::string& result) {
	const size_t read_buff_size = 4096;
	char read_buff[read_buff_size];
	ssize_t rd_bytes;

	rd_bytes = read(client_socket->get_fd(), read_buff, read_buff_size);
	if (rd_bytes < 0) {
		return Status(std::string("read() failed"), rd_bytes);
	}
	if (rd_bytes == 0) {
		return Status("EOF", rd_bytes);
	}
	read_buff[rd_bytes] = 0;
	result = std::string(read_buff);
	std::cout << CYAN300 << "REQUEST:\n" << read_buff << RESET << std::endl;
	return Status();
}

Status Server::request_handler(const ClientSocket* client_socket, t_request& req) {
	Status status;
	std::string request_string;

	status = read_request(client_socket, request_string);
	if (!status) {
		return Status("Error in Server::request_handler(): " + status.msg());
	}
	req = request_parser(request_string);
	return Status();
}

Status Server::response_handler(const ClientSocket* client_socket, const t_request& request) {
	ServerResponse resp(request, _configs[0]);
	std::string res = resp.generate_response();

	if (write(client_socket->get_fd(), res.c_str(), res.size()) < 0) {
		return Status(strerror(errno));
	}

	return Status();
}

Status Server::create_server_socket_manager(const std::string& host, int port) {
	Status status;
	ServerSocketManager* manager;

	manager = new ServerSocketManager(host, port, &_event);
	status = manager->start();
	if (!status) {
		delete manager;
		return status;
	}
	_server_socket_managers.insert(std::make_pair(manager->get_server_socket()->get_fd(), manager));
	return Status();
}

Status Server::create_sockets_from_config(const t_config& server_config) {
	const size_t amount_of_addresses = server_config.listen.size();
	Status status;

	for (size_t i = 0; i < amount_of_addresses; ++i) {
		const std::string& host = server_config.listen[i].host;
		int port = server_config.listen[i].port;

		status = create_server_socket_manager(host, port);
		if (!status) {
			return status;
		}
		// print_debug_addr(host, port);
	}
	return Status();
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
	return Status();
}

Status Server::find_server_socket_manager(
	int server_socket_fd, std::map<int, ServerSocketManager*>::iterator& search_result) {
	search_result = _server_socket_managers.find(server_socket_fd);
	if (search_result == _server_socket_managers.end()) {
		return Status("Server failed to find ServerSocketManager");
	}
	return Status();
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
