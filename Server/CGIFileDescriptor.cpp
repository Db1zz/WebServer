#include "CGIFileDescriptor.hpp"

#include "ClientSocket.hpp"

CGIFileDescriptor::CGIFileDescriptor(int fd, ClientSocket* client_socket)
	: FileDescriptor(FileDescriptor::CGIFD, fd), _client_socket(client_socket) {
}

ClientSocket* CGIFileDescriptor::get_client_socket() {
	return _client_socket;
}