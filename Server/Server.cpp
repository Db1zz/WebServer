#include "Server.hpp"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <cassert>
#include <csignal>
#include <cstdio>
#include <sstream>

#include "CGIFileDescriptor.hpp"
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
#include "ServerUtils.hpp"

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
	FileDescriptor* fd = static_cast<FileDescriptor*>(event.data.ptr);
	return find_server_socket_manager(fd->get_fd()) != NULL;
}

Status Server::handle_new_connection_event(const epoll_event& connection_event) {
	Status status;
	FileDescriptor* fd;

	fd = static_cast<FileDescriptor*>(connection_event.data.ptr);
	ServerSocketManager* manager = find_server_socket_manager(fd->get_fd());
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

Status Server::create_cgi_process(ClientSocket* client_socket) {
	ConnectionContext* connection_context = client_socket->get_connection_context();
	ServerSocketManager* server_socket_manager = find_server_socket_manager(client_socket->get_server_fd());
	t_request& request = connection_context->request;
	pid_t cgi_process;

	// fd[0] == read, fd[1] == write
	int server_read_pipe[2]; // this pipe is used to read stream of data from a client
	// int stdout_pipe[2];

	if (pipe(server_read_pipe) < 0) {
		perror("pipe");
		return Status("pipe() failed in create_cgi_process");
	}

	cgi_process = fork();
	if (cgi_process < 0) {
		perror("fork");
		close(server_read_pipe[0]);
		close(server_read_pipe[1]);
		return Status("fork() failed in create_cgi_process");
	}

	if (cgi_process == 0) {
		std::string cgi_bin_path;
		std::string cgi_bin_filename;
		std::string script_path = "." + request.uri_path;
		if (server_utils::get_cgi_bin(request.uri_path, server_socket_manager->get_server_config(), cgi_bin_path) == false) {
			std::cout << "failed to obtain cgi_bin\n";
			exit(127); // TODO idk what to do
		}
		server_utils::get_filename(cgi_bin_path, cgi_bin_filename);

		std::vector<std::string> argv_strs;
		argv_strs.push_back(cgi_bin_filename); // argv[0]
		argv_strs.push_back(script_path); // argv[1]
		std::vector<char*> argv;
		for (size_t i = 0; i < argv_strs.size(); ++i) {
			argv.push_back(const_cast<char*>(argv_strs[i].c_str()));
		}
		argv.push_back(NULL);

		std::vector<std::string> env_strs;
		{
			std::stringstream content_length;
			content_length << request.content_length;
			env_strs.push_back(std::string("REQUEST_METHOD=") + request.method);
			env_strs.push_back(std::string("CONTENT_LENGTH=") + content_length.str());
			env_strs.push_back(std::string("CONTENT_TYPE="));
			env_strs.push_back(std::string("SERVER_PROTOCOL=HTTP/1.1"));
			env_strs.push_back(std::string("SCRIPT_NAME="));
			env_strs.push_back(std::string("PATH_INFO="));
			env_strs.push_back(std::string("QUERY_STRING="));
			env_strs.push_back(std::string("REMOTE_ADDR="));
			env_strs.push_back(std::string("SERVER_NAME="));
			env_strs.push_back(std::string("SERVER_PORT="));
			env_strs.push_back(std::string("HTTP_USER_AGENT="));
			env_strs.push_back(std::string("HTTP_ACCEPT="));
			env_strs.push_back(std::string("SERVER_SOFTWARE=unravelThePuzzle"));
			env_strs.push_back(std::string("AUTH_TYPE=Basic"));
		}
		std::vector<char*> envp;
		for (size_t i = 0; i < env_strs.size(); ++i) {
			envp.push_back(const_cast<char*>(env_strs[i].c_str()));
		}
		envp.push_back(NULL);

		if (dup2(server_read_pipe[1], STDOUT_FILENO) < 0) {
			perror("dup2 in cgi child");
			exit(127);
		}

		close(server_read_pipe[0]);
		close(server_read_pipe[1]);

		execve(cgi_bin_path.c_str(), argv.data(), envp.data());

		perror("execve");
		exit(127);
	}
	close(server_read_pipe[1]);
	CGIFileDescriptor* descriptor = new CGIFileDescriptor(server_read_pipe[0], client_socket);
	connection_context->descriptors.insert(std::make_pair(descriptor->get_fd(), descriptor));

	_event.add_event(SERVER_EVENT_CLIENT_EVENTS, descriptor);
	return Status::OK();
}

Status Server::handle_cgi_request(ClientSocket* client_socket, int event_fd) {
	Status status;
	ConnectionContext* connection_context = client_socket->get_connection_context();
	if (connection_context->parser.is_body_parsed()) {
		status = receive_request_body_chunk(client_socket);
	}
	if (connection_context->cgi_started == false &&
		connection_context->request.is_request_ready()) {
		status = create_cgi_process(client_socket);
		if (!status) {
			_server_logger.log_error("Server::handle_cgi_request",
										std::string("create_cgi_process failed with error: '") +
											"TODO: Status error code!!!" + "'");
			return status;
		}
		connection_context->cgi_started = true;
	}

	return Status::OK();
}

Status Server::handle_normal_request(ClientSocket* client_socket) {
	ConnectionContext* connection_context = client_socket->get_connection_context();
	ServerSocketManager* manager = NULL;
	Status status;

	if (connection_context->parser.is_finished() == false) {
		status = receive_request_body_chunk(client_socket);
	}
	if (status.code() != DataIsNotReady) {
		ConnectionContext* connection_context = client_socket->get_connection_context();
		status = response_handler(client_socket);
		if (!status) {
			return Status("response_handler() failed in Server::handle_event(): " + status.msg());
		}
		if (connection_context->request.is_request_ready()) {
			manager = find_server_socket_manager(client_socket->get_server_fd());
			_server_logger.log_access(
				*client_socket->get_host(), connection_context->request.method,
				connection_context->request.uri_path, manager->get_server_socket()->get_port());
			client_socket->reset_connection_context();
		}
	}
	return Status::OK();
}

Status Server::receive_request_header(ClientSocket* client_socket) {
	ConnectionContext* connection_context = client_socket->get_connection_context();
	Status status;
	std::string buffer;
	int rd_bytes = 0;

	status = read_data(client_socket, buffer, rd_bytes);
	if (!status) {
		_server_logger.log_error("Server::receive_request_header", "failed to read data");
		return status;
	}
	status = connection_context->parser.parse_header(buffer, connection_context->buffer);
	if (!status) {
		_server_logger.log_error("Server::receive_request_header", "failed to parse header");
		return status;
	}

	return status;
}

Status Server::receive_request_body_chunk(ClientSocket* client_socket) {
	ConnectionContext* connection_context = client_socket->get_connection_context();
	Status status;
	int rd_bytes = 0;

	if (connection_context->buffer.empty() == false) {
		status = connection_context->parser.parse_body(connection_context->buffer);
		if (!status) {
			_server_logger.log_error("Server::receive_request_header", "failed to parse header");
			return status;
		}
		connection_context->buffer.clear();
	}

	if (connection_context->parser.is_body_parsed() == false) {
		status = read_data(client_socket, connection_context->buffer, rd_bytes);
		if (!status) {
			_server_logger.log_error("Server::receive_request_header", "failed to read data");
			return status;
		}
		status = connection_context->parser.parse_body(connection_context->buffer);
		if (!status) {
			_server_logger.log_error("Server::receive_request_header", "failed to parse header");
			return status;
		}
	}
	connection_context->buffer.clear();

	return status;
}

Status Server::close_connection_routine(FileDescriptor* fd) {
	if (fd->get_fd_type() == FileDescriptor::CGIFD) {
		CGIFileDescriptor* cgi_fd = static_cast<CGIFileDescriptor*>(fd);
		ClientSocket* client_socket = cgi_fd->get_client_socket();
		ConnectionContext* connection_context = client_socket->get_connection_context();
		std::map<int, FileDescriptor*>::iterator it =
			connection_context->descriptors.find(fd->get_fd());
		if (it != connection_context->descriptors.end()) {
			connection_context->descriptors.erase(it);
		}
		delete fd;
		std::cout << "cgi destroyed\n";
	} else if (fd->get_fd_type() == FileDescriptor::SocketFD) {
		ClientSocket* client_socket = static_cast<ClientSocket*>(fd);
		ServerSocketManager* manager = find_server_socket_manager(client_socket->get_server_fd());
		if (!manager) {
			return Status("Server cannot find a server to close connection with");
		}
		manager->close_connection_with_client(client_socket->get_fd());
		std::cout << "destroy client\n";
	}
	return Status::OK();
}

Status Server::cgi_fd_routine(CGIFileDescriptor* cgi_fd) {
	Status status;
	size_t buffer_size = 1000000;
	char buffer[buffer_size];
	ConnectionContext* connection_context = cgi_fd->get_connection_context();
	ssize_t rd_bytes = read(cgi_fd->get_fd(), buffer, buffer_size);
	if (rd_bytes == 0) { // GG EOF
		_event.remove_event(cgi_fd->get_fd());
		cgi_fd->close_fd();
		// return response ??
		std::cout << "DONE WITH CGI\n"; // never goes here
		return Status::OK();
	}

	if (rd_bytes < 0) {
		_server_logger.log_error("Server::cgi_fd_routine", "failed to read data from a cgi pipe");
		return Status::InternalServerError();
	}

	std::string content;
	content.append(buffer, rd_bytes);

	if (connection_context->opt_cgi_parser == NULL) {
		connection_context->opt_cgi_parser = new CGIResponseParser(&connection_context->request, &_server_logger);
	}

	status = connection_context->opt_cgi_parser->parse(content);
	if (!status) {
		_server_logger.log_error("Server::cgi_fd_routine",
								 "failed to parse CGI response");
	}
	std::cout << "DATA: " << connection_context->request.content_data.front().data << std::endl;

	return status;
}

Status Server::client_socket_routine(ClientSocket* client_socket) {
	Status status;
	ConnectionContext* connection_context = client_socket->get_connection_context();

	if (connection_context->state == ConnectionState::IDLE) {
		connection_context->state = ConnectionState::RECEIVING_REQUEST_HEADER_FROM_CLIENT;
	}
	if (connection_context->state == ConnectionState::RECEIVING_REQUEST_HEADER_FROM_CLIENT) {
		status = receive_request_header(client_socket);
		if (!status) {
			_server_logger.log_error("Server::handle_request_event",
									 "failed to receive request header");
			return status;
		}
		if (connection_context->parser.is_header_parsed() == true) {
			if (connection_context->parser.is_cgi_request() == true) {
				connection_context->state = ConnectionState::HANDLE_CGI_REQUEST;
			} else {
				connection_context->state = ConnectionState::HANDLE_NORMAL_REQUEST;
			}
		}
	}
	if (connection_context->state == ConnectionState::HANDLE_CGI_REQUEST) {
		status = handle_cgi_request(client_socket, 123);
	} else if (connection_context->state == ConnectionState::HANDLE_NORMAL_REQUEST) {
		status = handle_normal_request(client_socket);
	}
	return status;
}

Status Server::handle_request_event(const epoll_event& request_event) {
	Status status;
	FileDescriptor* fd = static_cast<FileDescriptor*>(request_event.data.ptr);

	if (request_event.events & (EPOLLERR | EPOLLRDHUP)) {
		status = close_connection_routine(fd);
	} else if (request_event.events & EPOLLIN) {
		if (fd->get_fd_type() == FileDescriptor::CGIFD) {
			status = cgi_fd_routine(static_cast<CGIFileDescriptor*>(fd));
		} else {
			status = client_socket_routine(static_cast<ClientSocket*>(fd));
		}
		if (!status) {
			_server_logger.log_error("Server::handle_request_event", "failed to handle request");
			return status;
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
