#include "Server.hpp"

#include <string.h>
#include <netdb.h> /* getnameinfo() */
#include <cstdio>
#include <errno.h>

Server::Server()
	: IServer(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY, 10)
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
	std::cout << GREEN400 << "----LISTENING AT PORT 80----" << RESET
	<< std::endl;
	while (true) {
		amount_of_events = _event.wait_event(-1);
		handle_event(amount_of_events);
	}
}

void Server::init() {
	_event.add_event(EPOLLIN | EPOLLOUT, get_socket()->get_fd());
}

void Server::handle_event(int amount_of_events) {
	for (int i = 0; i < amount_of_events; ++i) {
		if (_event[i]->data.fd == get_socket()->get_fd()) {
			// Accept connection and add new event
			accept_new_connection(_event[i]->data.fd);
		} else {
			// Send response on event
		}
	}
}

void Server::accept_new_connection(int new_connection_fd) {
	struct sockaddr cl_sockaddr;
	socklen_t cl_len = sizeof(cl_sockaddr);
	int cl_fd;

	cl_fd = accept(new_connection_fd, &cl_sockaddr, &cl_len);
	if (cl_fd < 0) {
		std::runtime_error("accept() failed: " + std::string(strerror(errno)));
	}
	_event.add_event(EPOLLIN | EPOLLOUT, cl_fd);
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
		"(host: %s, port: %s)\n", cl_fd, hbuff, sbuff);
}