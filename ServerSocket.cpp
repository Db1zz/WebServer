#include "ServerSocket.hpp"

ServerSocket::ServerSocket(int domain, int service, int protocol, int port, u_long interface): ASocket(domain, service, protocol, port, interface) {
	if (start_connection(get_socket(), get_address()) < 0)
		throw CannotConnectException();
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
// }

ServerSocket::~ServerSocket() {}

int ServerSocket::start_connection(int socket, sockaddr_in address) {
	return bind(socket, (struct sockaddr*)&address, sizeof(address));
}
