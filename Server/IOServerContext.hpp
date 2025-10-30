#ifndef SERVER_IO_SERVER_CONTEXTP_HPP_
#define SERVER_IO_SERVER_CONTEXTP_HPP_

#include "IIOContext.hpp"

class ServerSocketManager;

class IOServerContext : public IIOContext {
	public:
	// IOServerContext() {}
	void reset() {};

	ServerSocketManager* server_socket_manager;
};

#endif // SERVER_IO_SERVER_CONTEXTP_HPP_