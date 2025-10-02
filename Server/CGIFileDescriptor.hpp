#ifndef SERVER_CGI_FILE_DESCRIPTOR_HPP_
#define SERVER_CGI_FILE_DESCRIPTOR_HPP_

#include "FileDescriptor.hpp"
#include "ConnectionContext.hpp"

class ClientSocket;
class CGIFileDescriptor : public FileDescriptor {
public:
	CGIFileDescriptor(int fd, ClientSocket* client_socket);
	ClientSocket* get_client_socket();

	ConnectionContext* get_connection_context();
	void reset_connection_context();

   private:
	ConnectionContext _connection_context;
	ClientSocket* _client_socket;
};

#endif // SERVER_CGI_FILE_DESCRIPTOR_HPP_