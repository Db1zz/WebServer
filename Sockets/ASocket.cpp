#include "ASocket.hpp"

ASocket::ASocket(int domain, int service, int protocol, int port, u_long interface) {
	_address.sin_family = domain;
	_address.sin_port = htons(port);
	_address.sin_addr.s_addr = htonl(interface);

	_socket = socket(domain, service, protocol);
	//test connection ?
}

ASocket::ASocket(const ASocket &other) {
	*this = other;
}

ASocket &ASocket::operator = (const ASocket &other) {
	if (this != &other) {
		//assign;
	}
	return (*this);
}

ASocket::~ASocket() {}

/*getters*/
int ASocket::get_socket() const {
	return _socket;
}

struct sockaddr_in ASocket::get_address() {
	return _address;
}

/*setters*/
void ASocket::set_socket(int socket) {
	_socket = socket;
}

void ASocket::set_address(sockaddr_in address) {
	_address = address;
}

void ASocket::test_connection(int connection) {
	if (connection < 0)
		throw CannotConnectException();
}