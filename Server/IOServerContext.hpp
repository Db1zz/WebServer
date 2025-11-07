#ifndef SERVER_IO_SERVER_CONTEXTP_HPP_
#define SERVER_IO_SERVER_CONTEXTP_HPP_

#include "IIOContext.hpp"
#include "ServerSocketManager.hpp"

class IOServerContext : public IIOContext {
   public:
	IOServerContext(ServerSocketManager* server_socket_manager)
		: server_socket_manager(server_socket_manager) {}

	~IOServerContext() { delete server_socket_manager; }

	void reset() {};

	ServerSocketManager* server_socket_manager;
};

#endif // SERVER_IO_SERVER_CONTEXTP_HPP_