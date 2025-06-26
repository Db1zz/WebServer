#include "ServerSocket.hpp"

#include <unistd.h>
#include <cstdlib>
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

ServerSocket::ServerSocket(std::string host, int port)
	: _host(host), _port(port)
{
	_address_conf.sin_family = AF_INET;
	_address_conf.sin_port = htons(port);
	_address_conf.sin_addr.s_addr = inet_addr(_host.c_str());
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	is_socket_created(_socket_fd);
}

ServerSocket::ServerSocket(ServerSocket &other) {
	*this = other;
}

ServerSocket::~ServerSocket() {
	if (_socket_fd >= 0) {
		std::cout << "[ServerSocket] Destroying socket - " << _host << ":" << _port << std::endl;
		close_socket();
	}
}

ServerSocket &ServerSocket::operator=(ServerSocket &other) {
	if (this != &other) {
		if (_socket_fd >= 0) {
			close_socket();
		}
		_socket_fd = other._socket_fd;
		_address_conf = other._address_conf;
		_port = other._port;
		_host = other._host;
		other._socket_fd = -1;
	}
	return (*this);
}

/* getters */
int ServerSocket::get_fd() const {
	return _socket_fd;
}

struct sockaddr_in ServerSocket::get_address() {
	return _address_conf;
}

/* setters */
void ServerSocket::set_socket(int socket) {
	_socket_fd = socket;
}

void ServerSocket::set_address_conf(sockaddr_in addres_conf) {
	_address_conf = addres_conf;
}

void ServerSocket::set_opt(int opt, bool to_set, int level) {
	setsockopt(_socket_fd, level, opt, &to_set, sizeof(to_set));
}

/* general functions */
void ServerSocket::close_socket() {
	close(_socket_fd);
}

/* private functions */
void ServerSocket::is_socket_created(int socket_fd) {
	if (socket_fd < 0) {
		throw std::runtime_error("socket() failed: " + std::string(strerror(errno)));
	}
}

int ServerSocket::start_connection() {
	int status;
	status = bind(_socket_fd, (struct sockaddr*)&_address_conf, sizeof(_address_conf));
	is_binded(status);
	status = listen(_socket_fd, SOCKET_DEFAULT_MAX_CONNECTIONS);
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
