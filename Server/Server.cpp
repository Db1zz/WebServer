#include "Server.hpp"

#include <errno.h>
#include <netdb.h> /* getnameinfo() */
#include <string.h>

#include <cstdio>

Server::Server(std::vector<t_config> configs)
	: IServer(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY, 10), _configs(configs) {
	init();
}

Server::~Server() {
	// nothing to destroy :< living in peace <3
}

void Server::launch() {
	int amount_of_events = 0;

	/*
		TODO:
			Port shouldn't be static, modify cout later.
	*/
	get_socket()->start_connection();
	std::cout << GREEN400 << "----LISTENING AT PORT 80----" << RESET
			  << std::endl;
	while (true) {
		amount_of_events = _event.wait_event(2.5);
		handle_event(amount_of_events);
	}
}

void Server::init() {
	int yes = 1;

	_event.add_event(EPOLLIN | EPOLLOUT, get_socket()->get_fd());
	setsockopt(get_socket()->get_fd(), SOL_SOCKET, SO_REUSEADDR, &yes,
			   sizeof(yes));
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

void Server::handle_event(int amount_of_events) {
	for (int i = 0; i < amount_of_events; ++i) {
		const epoll_event &request_event = *_event[i];

		if (request_event.data.fd == get_socket()->get_fd()) {
			// Accept connection and add new event
			accept_new_connection(request_event.data.fd);
		} else {
			// handle request and generate response

			t_request req;
			request_handler(request_event, req);
			response_handler(request_event, req);
			close(request_event.data.fd);
		}
	}
}

std::string Server::read_request(
	const epoll_event &request_event) {
	const size_t read_buff_size = 4096;
	char read_buff[read_buff_size];
	ssize_t rd_bytes;

	rd_bytes = read(request_event.data.fd, read_buff, read_buff_size);
	if (rd_bytes < 0) {
		std::runtime_error("read() failed!");
		close(request_event.data.fd);
	}
	read_buff[rd_bytes] = '\0';
	return read_buff;
}

Status Server::request_handler(
	const epoll_event &request_event, t_request &req) {
	std::string request = read_request(request_event);
	req = request_parser(request);
	// request_validator();
	return Status();
}

void Server::response_handler(const epoll_event &request_event,
							  const t_request &request) {
	ServerResponse resp(request, _configs[0]);
	std::string res = resp.generate_response();
	write(request_event.data.fd, res.c_str(), res.size());
}

std::string Server::response_generator(/* TODO: add args*/) {
	std::string response_str;
	response_str = "some response";
	return response_str;
}

void Server::accept_new_connection(int new_connection_fd) {
	struct sockaddr cl_sockaddr;
	socklen_t cl_len = sizeof(cl_sockaddr);
	int cl_fd;

	cl_fd = accept(new_connection_fd, &cl_sockaddr, &cl_len);
	if (cl_fd < 0) {
		std::runtime_error("accept() failed: " + std::string(strerror(errno)));
	}
	_event.add_event(EPOLLIN, cl_fd);
	announce_new_connection(cl_sockaddr, cl_fd);
}

void Server::announce_new_connection(const struct sockaddr &cl_sockaddr,
									 int cl_fd) {
	char hbuff[NI_MAXHOST];
	char sbuff[NI_MAXSERV];
	int status = 0;

	status =
		getnameinfo(&cl_sockaddr, sizeof(cl_sockaddr), sbuff, sizeof(sbuff),
					hbuff, sizeof(hbuff), NI_NUMERICHOST | NI_NUMERICSERV);
	if (status != 0) {
		std::runtime_error("getnameinfo() failed");
	}
	printf(
		"Accepted new connection on descriptor %d\n"
		"(host: %s, addr: %s)\n",
		cl_fd, hbuff, sbuff);
}