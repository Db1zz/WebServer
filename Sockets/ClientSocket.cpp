#include "ClientSocket.hpp"

ClientSocket::ClientSocket(int domain, int service, int protocol, int port, u_long interface): ASocket(domain, service, protocol, port, interface) {
	if (start_connection(get_socket(), get_address()) < 0)
		throw CannotConnectException();
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

int ClientSocket::start_connection(int socket, sockaddr_in address) {
	return connect(socket, (struct sockaddr*)&address, sizeof(address));
}
