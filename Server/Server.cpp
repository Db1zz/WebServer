#include "Server.hpp"

#include <string.h>

Server::Server()
	: IServer(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY, 10)
{
	init();
}

Server::~Server() {
}

void Server::init() {
	event.add_event(EPOLLIN | EPOLLOUT, get_socket()->get_fd());
}

void Server::accept() {

}

void Server::process_handle() {

}

void Server::respond() {

}

void Server::launch() {
	std::cout << GREEN400 << "----LISTENING AT PORT 80----" << RESET
	<< std::endl;
	while (true) {
		// accept();
		// process_handle();
		// respond();
	}
}
