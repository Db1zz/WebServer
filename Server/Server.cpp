#include "Server.hpp"

#include <string.h>

Server::Server()
	: IServer(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY, 10)
{
	init();
}

Server::~Server() {
	// nothing to destroy :< blyat
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
		} else {
			// Send response on event
		}
	}
}
