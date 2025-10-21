#ifndef SERVER_CLIENT_SOCKET_HPP_
#define SERVER_CLIENT_SOCKET_HPP_

#include <map>

#include "ConnectionContext.hpp"
#include "Socket.hpp"

typedef s_config t_config;

class ClientSocket : public Socket {
   public:
	ClientSocket(const t_config* server_config, ServerLogger* server_logger);
	~ClientSocket();

	void set_server_fd(int server_fd);
	int get_server_fd();

	ConnectionContext* get_connection_context();
	void reset_connection_context();

   private:
	const t_config* _server_config;
	ConnectionContext _connection_context;
	FileDescriptor _server_fd;
};

#endif // SERVER_CLIENT_SOCKET_HPP_
