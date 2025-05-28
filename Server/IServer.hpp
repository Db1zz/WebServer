#ifndef SERVER_ISERVER_HPP
#define SERVER_ISERVER_HPP

#include <iostream>

#include "../Utilities/colors.hpp"
#include "ServerSocket.hpp"

class IServer {
   public:
	IServer(int domain, int service, int protocol, int port, u_long interface,
			int backlog);
	virtual ~IServer();
	virtual void launch() = 0;
	ServerSocket* get_socket();

   private:
	ServerSocket* _server_socket;
	virtual void server_accept() = 0;
	virtual void server_process_handle() = 0;
	virtual void server_respond() = 0;
};

#endif	// SERVER_ISERVER_HPP
