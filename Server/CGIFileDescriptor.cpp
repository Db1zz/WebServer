#include "CGIFileDescriptor.hpp"

#include "ClientSocket.hpp"

CGIFileDescriptor::CGIFileDescriptor(int fd, ClientSocket* client_socket)
	: FileDescriptor(FileDescriptor::CGIFD, fd),
	  _client_socket(client_socket),
	  _connection_context(client_socket->get_connection_context()->server_config) {
}

ClientSocket* CGIFileDescriptor::get_client_socket() {
	return _client_socket;
}

void CGIFileDescriptor::reset_connection_context() {
	_connection_context.reset();
}

ConnectionContext* CGIFileDescriptor::get_connection_context() {
	return &_connection_context;
}