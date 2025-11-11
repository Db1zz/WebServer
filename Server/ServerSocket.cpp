#include "ServerSocket.hpp"

#include <arpa/inet.h>
#include <errno.h>

#include "ClientSocket.hpp"
#include "Exceptions/SystemException.hpp"

ServerSocket::ServerSocket(const std::string& host, int port) : Socket() {
	_socket_type = Socket::SERVER_SOCKET;
	_host = host;
	_port = port;
	struct sockaddr_in* sockaddr = (struct sockaddr_in*) &_sockaddr;
	_socklen = sizeof(*sockaddr);
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons(port);
	sockaddr->sin_addr.s_addr = inet_addr(_host.c_str());
}

void ServerSocket::open_socket() {
	create_server_socket();
	set_socket_option(kReuseAddr, kSet);

	if (bind(get_fd(), &_sockaddr, _socklen) < 0) {
		throw SystemException(LOG_INFO(), "bind()" + std::string(strerror(errno)));
	}

	if (listen(get_fd(), SOCKET_DEFAULT_MAX_CONNECTIONS) < 0) {
		throw SystemException(LOG_INFO(), "listen()" + std::string(strerror(errno)));
	}
}

void ServerSocket::accept_connection(ClientSocket& empty_client_socket) {
	struct sockaddr sockaddr;
	socklen_t socklen = sizeof(sockaddr);

	empty_client_socket.set_socket(accept(get_fd(), &sockaddr, &socklen), &sockaddr, socklen);
	if (empty_client_socket.get_fd() < 0) {
		throw SystemException(LOG_INFO(), strerror(errno));
	}
	empty_client_socket.set_server_fd(get_fd());
}

void ServerSocket::create_server_socket() {
	set_fd(socket(AF_INET, SOCK_STREAM, 0));
	if (get_fd() < 0) {
		throw SystemException(LOG_INFO(), "socket()" + std::string(strerror(errno)));
	}
}