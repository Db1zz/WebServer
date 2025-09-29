#ifndef SERVER_CLIENT_SOCKET_HPP_
#define SERVER_CLIENT_SOCKET_HPP_

#include "Socket.hpp"
#include "ServerRequest.hpp"
#include "RequestParser/ServerRequestParser.hpp"

struct ClientConnectionContext {
	t_request request;
	ServerRequestParser parser;

	ClientConnectionContext();
	void reset();
};

class ClientSocket : public Socket {
   public:
	ClientSocket();
	~ClientSocket();

	void set_server_fd(int server_fd);
	int get_server_fd();

	ClientConnectionContext* get_connection_context();
	void reset_connection_context();

   private:
	ClientConnectionContext _connection_context;
	int _server_fd;
};

#endif // SERVER_CLIENT_SOCKET_HPP_
