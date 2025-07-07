#include "ServerSocket.hpp"

#include <arpa/inet.h>
#include <errno.h>

ServerSocket::ServerSocket(const std::string &host, int port)
{
	struct sockaddr_in *sockaddr = (struct sockaddr_in *)&_sockaddr;
	_socklen = sizeof(*sockaddr);

	_host = host;
	_port = port;

	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons(port);
	sockaddr->sin_addr.s_addr = inet_addr(_host.c_str());
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	is_socket_created(_socket_fd);
}

Status ServerSocket::listen_for_connections() {
	if (bind(_socket_fd, &_sockaddr, _socklen) < 0) {
		return Status("bind() ", strerror(errno));
	}
	if (listen(_socket_fd, SOCKET_DEFAULT_MAX_CONNECTIONS)) {
		return Status("listen() ", strerror(errno));
	}
	return Status();
}

/* private functions */
void ServerSocket::is_socket_created(int socket_fd) {
	if (socket_fd < 0) {
		throw std::runtime_error("socket() failed: " + std::string(strerror(errno)));
	}
}

Status ServerSocket::accept_connection(Socket &empty_socket) {
	struct sockaddr sockaddr;
	socklen_t socklen;
	int fd;

	fd = accept(_socket_fd, &sockaddr, &socklen);
	if (fd < 0) {
		return Status(strerror(errno));
	}

	empty_socket.set_sockaddr(&sockaddr, socklen);

	return Status();
}