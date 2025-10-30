#ifndef SERVER_CLIENT_SOCKET_HPP_
#define SERVER_CLIENT_SOCKET_HPP_

#include <map>

#include "Socket.hpp"
#include "ServerConfig.hpp"

class ServerLogger;
class CGIFileDescriptor;

class ClientSocket : public Socket {
   public:
	ClientSocket(const t_config* server_config);
	~ClientSocket();

	void set_server_fd(int server_fd);
	int get_server_fd();

   private:
	const t_config* _server_config;

	FileDescriptor _server_fd;
};

#endif // SERVER_CLIENT_SOCKET_HPP_
