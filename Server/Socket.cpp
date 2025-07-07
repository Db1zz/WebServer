#include "Socket.hpp"

#include <unistd.h>
#include <cstdlib>
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <iostream>

Socket::Socket()
	: _port(-1), _socket_fd(-1), _socklen(-1) {}

Socket::Socket(Socket &other) {
	*this = other;
}

Socket::~Socket() {
	if (_socket_fd >= 0) {
		std::cout << "[Socket] Destroying socket - " << _host << ":" << _port << std::endl;
		close_socket();
	}
}

Socket &Socket::operator=(Socket &other) {
	if (this != &other) {
		if (_socket_fd >= 0) {
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

const struct sockaddr *Socket::get_address() const {
	return &_sockaddr;
}

const std::string *Socket::get_host() const {
	return &_host;
}

int Socket::get_port() const {
	return _port;
}

socklen_t Socket::get_socklen() const {
	return _socklen;
}

/* setters */
void Socket::set_socket(int socket) {
	_socket_fd = socket;
}

void Socket::set_sockaddr(const struct sockaddr *sockaddr, socklen_t socklen) {
	_sockaddr = *sockaddr;
	_socklen = socklen;
}

Status Socket::set_opt(int opt, bool to_set, int level) {
	if (setsockopt(_socket_fd, level, opt, &to_set, sizeof(to_set)) < 0) {
		return Status("setsockopt() ", strerror(errno));
	}
	return Status();
}

/* general functions */
Status Socket::close_socket() {
	if (close(_socket_fd) < 0) {
		return Status("close() ", strerror(errno));
	}
	return Status();
}
