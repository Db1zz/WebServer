#include "ServerSocket.hpp"

#include <arpa/inet.h>
#include <errno.h>

ServerSocket::ServerSocket(const std::string& host, int port) {
	_host = host;
	_port = port;
	struct sockaddr_in* sockaddr = (struct sockaddr_in*) &_sockaddr;
	_socklen = sizeof(*sockaddr);
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons(port);
	sockaddr->sin_addr.s_addr = inet_addr(_host.c_str());
}

Status ServerSocket::open_socket() {
	Status status;

	status = create_server_socket();
	if (!status) {
		return status;
	}

	status = set_socket_option(kReuseAddr, kSet);
	if (!status) {
		return status;
	}

	if (bind(_socket_fd, &_sockaddr, _socklen) < 0) {
		return Status("ServerSocket failed to bind socket: ", strerror(errno));
	}
	if (listen(_socket_fd, SOCKET_DEFAULT_MAX_CONNECTIONS) < 0) {
		return Status("ServerSocket failed to listen socket: ", strerror(errno));
	}

	return Status();
}

Status ServerSocket::accept_connection(ClientSocket& empty_client_socket) {
	struct sockaddr sockaddr;
	socklen_t socklen = sizeof(sockaddr);

	empty_client_socket.set_socket(accept(_socket_fd, &sockaddr, &socklen), &sockaddr, socklen);
	if (empty_client_socket.get_fd() < 0) {
		return Status(strerror(errno));
	}
	empty_client_socket.set_server_fd(_socket_fd);

	return Status();
}

Status ServerSocket::create_server_socket() {
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd < 0) {
		return Status("ServerSocket failed to create a socket: " + std::string(strerror(errno)));
	}
	return Status();
}
