#include "ClientSocket.hpp"

#include <stdexcept>
#include <errno.h>
#include <string.h>

ClientSocket::ClientSocket(int domain, int service, int protocol, int port, u_long interface): ASocket(domain, service, protocol, port, interface) {
	int status = start_connection();
	is_connected(status);
}

//TODO:
//create the copy constructor and assignment operator when incoming data format is agreed upon

// ClientSocket::ClientSocket(const ClientSocket &other)
// {
// 	*this = other;
// }

// ClientSocket &ClientSocket::operator = (const ClientSocket &other)
// {
// 	std::cout << "ClientSocket Copy assignment operator called" << std::endl;
// 	if (this != &other)
// 	{
// 		//assign;
// 	}
// 	return (*this);
// }

int ClientSocket::start_connection() {
	return connect(_socket_fd, (struct sockaddr*)&_address, sizeof(_address));
}

void ClientSocket::is_connected(int status) {
	if (status < 0) {
		throw std::runtime_error("connect() faled: " + std::string(strerror(errno)));
	}
}