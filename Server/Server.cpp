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
	return Status();
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

		find_server_socket_manager(client_socket->get_server_fd(), search);
		_server_logger.log_access(*client_socket->get_host(), request.method, request.uri_path,
								  search->second->get_server_socket()->get_port());
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

Status Server::request_parser(std::string request, t_request& requestStruct) {
	t_request newRequestStruct;
	Status status;
	newRequestStruct.mime_type = ""; // if there no mime found -> empty string
	std::stringstream iss(request);
	std::string extract;
	iss >> extract;
	newRequestStruct.method = extract; // if method is not GET DELETE POST -> error
	if (newRequestStruct.method.compare("GET") != 0 &&
		newRequestStruct.method.compare("DELETE") != 0 &&
		newRequestStruct.method.compare("POST") != 0) {
		status.set_status_line(405, "Method not allowed " + newRequestStruct.method);
		status.set_ok(false);
		return status;
	}
	iss >> extract;
	newRequestStruct.uri_path = extract;
	if (extract.find('.') != std::string::npos) {
		size_t dotPos = extract.find('.');
		size_t questionMarkPos = extract.find('?', dotPos);
		if (questionMarkPos != std::string::npos && questionMarkPos > dotPos) {
			newRequestStruct.mime_type = extract.substr(dotPos, questionMarkPos - dotPos);
			newRequestStruct.cgi_query_string = extract.substr(questionMarkPos + 1);
			newRequestStruct.uri_path =
				extract.substr(0, questionMarkPos); // update uri_path without query
		} else {
			newRequestStruct.mime_type = extract.substr(dotPos);
		}
	}
	else if (extract == "/")
		newRequestStruct.mime_type = ".html";
	iss >> extract; // if it's not HTTP/1.1 error
	if (extract.compare("HTTP/1.1") != 0) {
		status.set_status_line(400, "This server only supports HTTP/1.1");
		status.set_ok(false);
		return status;
	}
	while (std::getline(iss, extract) || extract != "\r") {
		std::cout << extract << "\n";
		if (extract.empty() || extract == "\r\n") break;
		if (extract.find("Host: ", 0) != std::string::npos)
			newRequestStruct.host = extract.substr(6);
		else if (extract.find("User-Agent: ", 0) != std::string::npos)
			newRequestStruct.user_agent = extract.substr(12);
		else if (extract.find("Accept: ", 0) != std::string::npos)
			newRequestStruct.accept = extract.substr(8);
		else if (extract.find("Accept-Language: ", 0) != std::string::npos)
			newRequestStruct.language = extract.substr(17);
		else if (extract.find("Connection: ", 0) != std::string::npos)
			newRequestStruct.connection = extract.substr(12);
		else if (extract.find("Content-Length: ") != std::string::npos)
			newRequestStruct.content_length = atol(extract.substr(16).c_str());
		else if (extract.find("Content-Type: ") != std::string::npos)
			newRequestStruct.content_type = extract.substr(14);
		if (!extract.empty() && extract[extract.size()-1] == '\r')
			extract.erase(extract.size()-1);
		if (extract.empty()) break; // End of headers

	}
	requestStruct = newRequestStruct;
	if (requestStruct.method.compare("POST") == 0 || requestStruct.method.compare("DELETE") == 0) {
		status = handle_post_or_delete(request, requestStruct);
		return status;
	}
	return Status();
}

std::string extract_filename(std::string extractee)
{
	std::string filename = "";
	size_t filename_pos = extractee.find("filename=\"");
	if (filename_pos != std::string::npos) {
		size_t start = filename_pos + 10; // length of 'filename="'
		size_t end = extractee.find("\"", start);
		if (end != std::string::npos) {
			filename = extractee.substr(start, end - start);
		}
	}
	return filename;
}

std::string extract_file_content(std::string request, std::string boundary)
{
	std::string file_content = "";
	std::string delimiter = "--" + boundary;
	size_t part_start = request.find(delimiter);
	if (part_start == std::string::npos)
		return file_content;

	// Move to the start of the part after the boundary line
	part_start += delimiter.length();
	// Skip possible \r\n after boundary
	if (request.substr(part_start, 2) == "\r\n")
		part_start += 2;

	// Find the end of the part headers
	size_t header_end = request.find("\r\n\r\n", part_start);
	if (header_end == std::string::npos)
		return file_content;

	// File content starts after the part headers
	size_t content_start = header_end + 4;

	// Find the next boundary (end of file content)
	size_t content_end = request.find(delimiter, content_start);
	if (content_end == std::string::npos)
		content_end = request.length();

	file_content = request.substr(content_start, content_end - content_start);

	// Remove possible trailing \r\n
	while (!file_content.empty() && (file_content[file_content.size()-1] == '\n' || file_content[file_content.size()-1] == '\r'))
		file_content.erase(file_content.size()-1);

	return file_content;
}

Status Server::handle_post_or_delete(std::string request, t_request& requestStruct)
{
	Status status;
	size_t boundaryPos = request.find("boundary=");
	if (boundaryPos != std::string::npos)
	{
		size_t boundaryStart = boundaryPos + 9; // length of "boundary="
		size_t boundaryEnd = request.find("\r\n", boundaryStart);
		if (boundaryEnd != std::string::npos) {
			requestStruct.bound = request.substr(boundaryStart, boundaryEnd - boundaryStart);
		}
	}
	else {
		status.set_status_line(400, "Bad request, boundary must be provided");
		status.set_ok(false);
		return status;
	}
	std::string filename = extract_filename(request);
	std::string tempFileContent;
	if (filename != "")
		tempFileContent = extract_file_content(request, requestStruct.bound);
	requestStruct.files[filename] = tempFileContent;
	// std::cout << "Filename: " << filename << "\n";
	// std::cout << "File Content: " << tempFileContent << "\n";
	// std::cout << "BOUND: " << requestStruct.bound << "\n";

	return status;
}

Status Server::read_request(const ClientSocket* client_socket, std::string& result) {
	const size_t read_buff_size = 4096;
	char read_buff[read_buff_size + 1];
	ssize_t rd_bytes;

	do {
		rd_bytes = read(client_socket->get_fd(), read_buff, read_buff_size);
		if (rd_bytes > 0) {
			read_buff[rd_bytes] = 0;
			result.append(read_buff);
		}
	} while (rd_bytes > 0);
	std::cout << CYAN300 << "REQUEST:\n" << result << RESET << std::endl;
	return Status();
}

Status Server::request_handler(const ClientSocket* client_socket, t_request& req) {
	Status status;
	Status parseStatus;
	std::string request_string;
	Status readStatus;

	readStatus = read_request(client_socket, request_string);
	if (!readStatus) {
		return Status("Error in Server::request_handler(): " + readStatus.msg());
	}
	parseStatus = request_parser(request_string, req);
	if (!parseStatus) {
		std::cout << "Continue with the next request\n";
	}
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
