#include "Socket.hpp"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

Socket::Socket() : _port(-1), _socket_fd(-1), _socklen(-1) {
}

Socket::Socket(Socket& other) {
	*this = other;
}

Socket::~Socket() {
	if (is_connected()) {
		close_socket();
	}
}

Socket& Socket::operator=(Socket& other) {
	if (this != &other) {
		if (is_connected()) {
			close_socket();
		}
		_socket_fd = other._socket_fd;
		_sockaddr = other._sockaddr;
		_port = other._port;
		_host = other._host;
		other._socket_fd = -1;
	}
	return (*this);
}

/* getters */
int Socket::get_fd() const {
	return _socket_fd;
}

const struct sockaddr* Socket::get_address() const {
	return &_sockaddr;
}

const std::string* Socket::get_host() const {
	return &_host;
}

int Socket::get_port() const {
	return _port;
}

socklen_t Socket::get_socklen() const {
	return _socklen;
}

/* setters */
void Socket::set_socket(int socket, const struct sockaddr* sockaddr, socklen_t socklen) {
	_sockaddr = *sockaddr;
	_socklen = socklen;
	_socket_fd = socket;

	set_host_ipv4_address_from_sockaddr();
	set_port_ipv4_from_sockaddr();
}

Status Socket::set_socket_option(SocketOption socket_option, SetMode mode) {
	int mode_int = static_cast<int>(mode);
	int socket_option_int;

	if (socket_option == kReuseAddr) {
		socket_option_int = SO_REUSEADDR;
	}

	if (_socket_fd < 0) {
		return Status("Socket failed to set option: socket is not created");
	}

	if (setsockopt(_socket_fd, SOL_SOCKET, socket_option_int, &mode_int, sizeof(mode_int)) < 0) {
		return Status("Socket failed to set option: ", strerror(errno));
	}
	return Status();
}

Status Socket::is_connected() const {
	if (_socket_fd < 0) {
		return Status("Socket fd is < 0");
	}
	return Status();
}

/* general functions */
Status Socket::close_socket() {
	if (close(_socket_fd) < 0) {
		return Status("close() ", strerror(errno));
	}
	_socket_fd = -1;
	return Status();
}

Status Socket::set_host_ipv4_address_from_sockaddr() {
	const struct sockaddr_in* ipv4_address =
		reinterpret_cast<const struct sockaddr_in*>(&_sockaddr);
	const unsigned char* octets =
		reinterpret_cast<const unsigned char*>(&ipv4_address->sin_addr.s_addr);
	const size_t amount_of_octets = sizeof(ipv4_address->sin_addr.s_addr);
	std::stringstream ss;

	for (size_t i = 0; i < amount_of_octets - 1; ++i) {
		ss << static_cast<int>(octets[i]) << '.';
	}
	ss << static_cast<int>(octets[amount_of_octets - 1]);

	try {
		_host = ss.str();
	} catch (const std::exception& e) {
		return Status(e.what());
	}

	return Status();
}

void Socket::set_port_ipv4_from_sockaddr() {
	const struct sockaddr_in* ipv4_address =
		reinterpret_cast<const struct sockaddr_in*>(&_sockaddr);

	_port = ipv4_address->sin_port;
}