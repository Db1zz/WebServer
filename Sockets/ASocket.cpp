#include "ASocket.hpp"

#include <unistd.h>
#include <cstdlib>
#include <stdexcept>
#include <errno.h>
#include <string.h>

ASocket::ASocket(int domain, int service, int protocol, int port, u_long interface) {
	_address.sin_family = domain;
	_address.sin_port = htons(port);
	_address.sin_addr.s_addr = htonl(interface);

	_socket_fd = socket(domain, service, protocol);
	is_socket_created(_socket_fd);
}

ASocket::ASocket(const ASocket &other) {
	*this = other;
}

ASocket &ASocket::operator=(const ASocket &other) {
	if (this != &other) {
		_socket_fd = other._socket_fd;
		_address = other._address;
	}
	return (*this);
}

ASocket::~ASocket() {}

/* getters */
int ASocket::get_fd() const {
	return _socket_fd;
}

struct sockaddr_in ASocket::get_address() {
	return _address;
}

/* setters */
void ASocket::set_socket(int socket) {
	_socket_fd = socket;
}

void ASocket::set_address(sockaddr_in address) {
	_address = address;
}

/* general functions */
void ASocket::close_socket() {
	close(_socket_fd);
}

/* private functions */
void ASocket::is_socket_created(int socket_fd) {
	if (socket_fd < 0)
		throw std::runtime_error("socket() failed: " + std::string(strerror(errno)));
}
