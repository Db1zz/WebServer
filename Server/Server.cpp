#include "Server.hpp"

#include <string.h>
#include <netdb.h> /* getnameinfo() */
#include <cstdio>
#include <errno.h>

Server::Server()
	: IServer(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, 10)
{
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
	std::cout << GREEN400 << "----LISTENING AT PORT 8080----" << RESET
	<< std::endl;
	while (true) {
		amount_of_events = _event.wait_event(2.5);
		handle_event(amount_of_events);
	}
}

void Server::init() {
	int yes = 1;

	_event.add_event(EPOLLIN | EPOLLOUT, get_socket()->get_fd());
	setsockopt(get_socket()->get_fd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
}

void Server::handle_event(int amount_of_events) {
	for (int i = 0; i < amount_of_events; ++i) {
		const epoll_event &request_event = *_event[i];

		if (request_event.data.fd == get_socket()->get_fd()) {
			// Accept connection and add new event
			accept_new_connection(request_event.data.fd);
		} else {
			// handle request and generate response
			std::vector<std::string> request;

			request = request_handler(request_event);
			request.size(); // suppress unused variable err
			response_handler(request_event);
		}
	}
}

std::vector<std::string> Server::read_request(const epoll_event &request_event) {
	const size_t read_buff_size = 1024;
	char read_buff[read_buff_size]; // TODO: read about what size of the header we can get
	ssize_t rd_bytes;

	rd_bytes = read(request_event.data.fd, read_buff, read_buff_size);
	if (rd_bytes < 0) {
		std::runtime_error("read() failed!");
		close(request_event.data.fd);
	}
	// TODO: add loop in which we're going to fill std::vector<std::string>
	std::cout << read_buff << std::endl;
	return std::vector<std::string>(); // return empty arr
}

std::vector<std::string> Server::request_handler(const epoll_event &request_event) {
	std::vector<std::string> request = read_request(request_event);

	/* 
		Goshan41k

		We can validate request during parsing or before/after it,
		depends on our implementation.

		Parser should return struct, which will be used later in response_handler(),
		right now I'm returning std::vector<std::string> cuz nothing is implemented and 
		i'm not sure how this struct will look like. 
	*/
	// parse_request()
	// request_validator();
	return request;
}

void Server::response_handler(const epoll_event &request_event /* second arg is a struct that was parsed in request_handler()*/) {
	char aboba[] = "aboba";
	// Generate response and send it to request_event.data.fd
	write(request_event.data.fd, "aboba", sizeof(aboba));
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

void Server::announce_new_connection(const struct sockaddr &cl_sockaddr, int cl_fd) {
	char hbuff[NI_MAXHOST];
	char sbuff[NI_MAXSERV];
	int status = 0;

	status = getnameinfo(&cl_sockaddr, sizeof(cl_sockaddr),
						sbuff, sizeof(sbuff),
						hbuff, sizeof(hbuff),
						NI_NUMERICHOST | NI_NUMERICSERV);
	if (status != 0) {
		std::runtime_error("genameinfo() failed");
	}
	printf("Accepted new connection on descriptor %d\n"
		"(host: %s, addr: %s)\n", cl_fd, hbuff, sbuff);
}