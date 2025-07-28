#include "Server.hpp"

#include <string.h>
#include <sstream>

#include <errno.h>
#include <cstdio>
#include <csignal>

namespace
{
	volatile std::sig_atomic_t g_signal_status = 0;
}

void sigint_handler(int signal) {
	g_signal_status = signal;
}

Server::Server(const std::vector<t_config> &configs)
	: _configs(configs), _server_max_fd_limit(SERVER_DEFAULT_MAX_CONNECTIONS), _server_amount_of_used_fds(0)
{
	Status status;

	_fd_data.resize(_server_max_fd_limit);
	status = create_multiple_servers();
	if (!status) {
		throw std::runtime_error(status.msg());
	}
}

Server::~Server() {
	destructor_destroy_sockets();
	destructor_close_all_connections_and_destroy_fd_data();
}

void Server::destructor_destroy_sockets() {
	for (size_t i = 0; i < _sockets.size(); ++i) {
		if (_sockets[i] == NULL) {
			continue;
		}
		delete _fd_data[_sockets[i]->get_fd()];
		_fd_data[_sockets[i]->get_fd()] = NULL;
		_event.remove_event(_sockets[i]->get_fd());
		delete _sockets[i];
	}
}

void Server::destructor_close_all_connections_and_destroy_fd_data() {
	for (size_t i = 0; i < _fd_data.size(); ++i) {
		if (_fd_data[i] == NULL) {
			continue;
		}
		close_connection(_fd_data[i]);
	}
}

Status Server::launch() {
	Status status;
	int amount_of_events = 0;

	std::signal(SIGINT, sigint_handler);
	status = start_servers();
	if (!status) {
		return status;
	}

	while (g_signal_status != SIGINT) {
		status = _event.wait_event(-1, &amount_of_events);
		if (!status && status.code() != EINTR) {
			throw std::runtime_error("wait_event() failed in Server::Launch(): " + status.msg());
		}

		if (amount_of_events > 0) {
			status = handle_event(amount_of_events);
			if (!status) {
				throw std::runtime_error("handle_event() failed in Server::Launch(): " + status.msg());
			}
		}
	}
	std::cout << "[Server] shutdown..." << std::endl;
	return status;
}

bool Server::is_a_new_connection(const epoll_event &event) {
	t_event_ctx *event_ctx = static_cast<t_event_ctx *>(event.data.ptr);
	int event_fd = event_ctx->socket->get_fd();

	for (size_t i = 0; i < _sockets.size(); ++i) {
		if (event_fd == _sockets[i]->get_fd()) {
			return true;
		}
	}
	return false;
}

Status Server::register_connection_server_event(Socket &new_connection_socket, const t_event_ctx *server_event_ctx) {
	Status status;
	t_event_ctx *new_event_ctx;
	int new_connection_fd;

	new_connection_fd = new_connection_socket.get_fd();
	new_event_ctx = new t_event_ctx(&new_connection_socket, server_event_ctx->data);
	_fd_data[new_connection_fd] = new_event_ctx;

	++_server_amount_of_used_fds;
	status = _event.add_event(SERVER_EVENT_CLIENT_EVENTS, new_connection_fd, new_event_ctx);
	return status;
}

Status Server::unregister_connection_server_event(int connection_fd) {
	Status status;

	delete _fd_data[connection_fd];
	_fd_data[connection_fd] = NULL;

	--_server_amount_of_used_fds;
	status = _event.remove_event(connection_fd);
	return status;
}

Status Server::set_connection_socket_nonblocking(Socket &socket) {
	if (fcntl(socket.get_fd(), F_SETFL, O_NONBLOCK) < 0) {
		return Status(strerror(errno));
	}
	return Status();
}

Status Server::accept_connection(const epoll_event &request_event) {
	Status status;
	Socket *new_socket;
	t_event_ctx *server_event_ctx;
	ServerSocket *server_socket;

	/*
		This error is not fatal, so server shouldn't exit.
	*/
	if (_server_amount_of_used_fds + 1 >= _server_max_fd_limit) {
		return Status("The maximum number of connections to the server has been exceeded", -1, true);
	}

	server_event_ctx = static_cast<t_event_ctx *>(request_event.data.ptr);
	server_socket = dynamic_cast<ServerSocket *>(server_event_ctx->socket);

	new_socket = new Socket;
	status = server_socket->accept_connection(*new_socket);
	if (!status) {
		return Status("Cannot accept new connection: " + status.msg());
	}
	printf("Accepted new connection on descriptor %d\n", server_socket->get_fd());

	set_connection_socket_nonblocking(*new_socket);
	status = register_connection_server_event(*new_socket, server_event_ctx);
	return status;
}

Status Server::close_connection(const epoll_event &request_event) {
	t_event_ctx *connection_event_ctx = static_cast<t_event_ctx *>(request_event.data.ptr);
	return close_connection(connection_event_ctx);
}

Status Server::close_connection(t_event_ctx *connection_event_ctx) {
	Status status;
	int connection_fd;

	std::cout << "[Server] Connection with the host " 
	<< *connection_event_ctx->socket->get_host() << ":" << connection_event_ctx->socket->get_port() << " has been closed\n";

	connection_fd = connection_event_ctx->socket->get_fd();
	// std::cout << "[Server] Socket[" << connection_fd << "] has been closed" << std::endl;
	delete connection_event_ctx->socket;
	status = unregister_connection_server_event(connection_fd);

	return status;
}

Status Server::handle_event(int amount_of_events) {
	Status status;

	for (int i = 0; i < amount_of_events; ++i) {
		const epoll_event &request_event = *_event[i];
		if (is_a_new_connection(request_event)) {
			status = accept_connection(request_event);
			if (!status) {
				return Status("accept_new_connection() failed in Server::handle_event(): " + status.msg());
			}
		} else {
			if (request_event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
				close_connection(request_event);
				continue;
			}
			if (request_event.events & EPOLLIN) {
				t_request request;
				status = request_handler(request_event, request);
				if (!status) {
					return Status("request_handler() failed in Server::handle_event(): " + status.msg());
				}
				status = response_handler(request_event, request);
				if (!status) {
					return Status("response_handler() failed in Server::handle_event(): " + status.msg());
				}
			}
		}
	}
	return Status();
}

t_request Server::request_parser(std::string request) {
	std::cout << request << "\n";
	t_request requestStruct;
	requestStruct.mime_type = "";  // if there no mime found -> empty string
	std::stringstream iss(request);
	std::string extract;
	iss >> extract;
	requestStruct.method = extract;
	iss >> extract;
	requestStruct.uri_path = extract;
	if (extract.find('.') != std::string::npos)
	requestStruct.mime_type = extract.substr(extract.find('.'));
	iss >> extract;	 // we ignore HTTP/1.1 for now
	while (std::getline(iss, extract) || extract != "\r") {
		if (extract.empty() || extract == "\r\n")
		break;
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

Status Server::read_request(const epoll_event &request_event, std::string &result) {
	const t_event_ctx *event_ctx = static_cast<t_event_ctx *>(request_event.data.ptr);
	const size_t read_buff_size = 4096;
	char read_buff[read_buff_size];
	ssize_t rd_bytes;

	rd_bytes = read(event_ctx->socket->get_fd(), read_buff, read_buff_size);
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

Status Server::request_handler(const epoll_event &request_event, t_request &req) {
	Status status;
	std::string request_string;

	status = read_request(request_event, request_string);
	if (!status) {
		return Status("Error in Server::request_handler(): " + status.msg());
	}
	req = request_parser(request_string);
	return Status();
}

Status Server::response_handler(const epoll_event &request_event, const t_request &request)
{
	const t_event_ctx *event_ctx = static_cast<t_event_ctx *>(request_event.data.ptr);
	ServerResponse resp(request, _configs[0]);
	std::string res = resp.generate_response();

	if (write(event_ctx->socket->get_fd(), res.c_str(), res.size()) < 0) {
		return Status(strerror(errno));
	}

	return Status();
}

std::string Server::response_generator(/* TODO: add args*/) {
	std::string response_str;
	response_str = "some response";
	return response_str;
}

Status Server::create_single_server(t_config &server_config, const std::string &host, int port) {
	const int yes = 1;
	ServerSocket *new_server_socket;
	int new_server_fd;
	t_event_ctx *new_event_ctx;

	new_server_socket = new ServerSocket(host, port);
	new_server_fd = new_server_socket->get_fd();
	new_event_ctx = new t_event_ctx(new_server_socket, &server_config);

	setsockopt(new_server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	_fd_data[new_server_fd] = new_event_ctx;
	_sockets.push_back(new_server_socket);
	++_server_amount_of_used_fds;
	return _event.add_event(SERVER_EVENT_SERVER_EVENTS, new_server_fd, new_event_ctx);
}

Status Server::create_single_server_with_multiple_addresses(t_config &server_config) {
	const size_t amount_of_addresses = server_config.listen.size();
	Status status;

	for (size_t i = 0; i < amount_of_addresses; ++i) {
		const std::string &host = server_config.listen[i].host;
		int port = server_config.listen[i].port;

		status = create_single_server(server_config, host, port);
		if (!status) {
			break;
		}

		print_debug_addr(host, port);
	}
	return status;
}

Status Server::create_multiple_servers() {
	Status status;

	for (size_t i = 0; i < _configs.size(); ++i) {
		t_config &config = _configs[i];
		try {
			status = create_single_server_with_multiple_addresses(config);
			if (!status) {
				return status;
			}
		} catch (const std::exception &e) {
			return Status(std::string("error in Server::create_sockets_from_configs(): ") + e.what());
		}
	}
	return status;
}

Status Server::start_servers() {
	const size_t amount_of_servers = _sockets.size();
	Status status;

	for (size_t i = 0; i < amount_of_servers; ++i) {
		status = _sockets[i]->listen_for_connections();
		if (!status) {
			return Status("Failed to start servers: " + status.msg());
		}
	}
	return status;
}

void Server::print_debug_addr(const std::string &address, int port) {
	std::cout << GREEN400 << "Listening at: " << address << ":" << port << RESET << std::endl;
}
