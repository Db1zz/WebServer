#include "Server.hpp"

Server::Server() : IServer(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY, 10) {
	launch();
}

void Server::server_accept() {
	struct sockaddr_in address = get_socket()->get_address();
	int addr_len = sizeof(address);
	_new_socket = accept(get_socket()->get_socket(),
						 (struct sockaddr *)&address, (socklen_t *)&addr_len);
	read(_new_socket, _buffer, 40000);
}

void Server::server_process_handle() {
	std::cout << "buffer: " << _buffer << std::endl;
}

void Server::server_respond() {
	std::cout << "responce in progress" << std::endl;
	write(_new_socket, "hello bratishki", strlen("hello bratishki"));
	close(_new_socket);
}

void Server::launch() {
	while (true) {
		std::cout << GREEN400 << "----LISTENING AT PORT 80----" << RESET
				  << std::endl;
		server_accept();
		server_process_handle();
		server_respond();
	}
}
