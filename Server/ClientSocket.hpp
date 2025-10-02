#ifndef SERVER_CLIENT_SOCKET_HPP_
#define SERVER_CLIENT_SOCKET_HPP_

#include "ConnectionContext.hpp"
#include "Socket.hpp"
#include <map>

class ClientSocket : public Socket {
   public:
	ClientSocket();
	~ClientSocket();

	void set_server_fd(int server_fd);
	int get_server_fd();

	ConnectionContext* get_connection_context();
	void reset_connection_context();

   private:
	ConnectionContext _connection_context;
	FileDescriptor _server_fd;
};

#endif // SERVER_CLIENT_SOCKET_HPP_
