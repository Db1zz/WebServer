#include "ServerSocket.hpp"

#include <errno.h>
#include <stdexcept>
#include <string.h>

ServerSocket::ServerSocket(int domain, int service, int protocol, int port, u_long interface, int max_connections)
	: ASocket(domain, service, protocol, port, interface), _max_connections(max_connections)
{
}

//TODO:
//create the copy constructor and assignment operator when incoming data format is agreed upon

// ServerSocket::ServerSocket(const ServerSocket &other)
// {
// 	*this = other;
// }

// ServerSocket &ServerSocket::operator = (const ServerSocket &other)
// {
// 	std::cout << "ServerSocket Copy assignment operator called" << std::endl;
// 	if (this != &other)
// 	{
// 		//assign;
// 	}
// 	return (*this);

int ServerSocket::start_connection() {
	int status;
	status = bind(_socket_fd, (struct sockaddr*)&_address, sizeof(_address));
	is_binded(status);
	status = listen(get_fd(), _max_connections);
	is_listening(status);
	return 0;
}

void ServerSocket::is_listening(int status) {
	if (status < 0) {
		throw std::runtime_error("listen() failed: " + std::string(strerror(errno)));
	}
}

void ServerSocket::is_binded(int status) {
	if (status < 0) {
		throw std::runtime_error("bind() failed: " + std::string(strerror(errno)));
	}
}
