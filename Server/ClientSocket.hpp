#ifndef SERVER_CLIENT_SOCKET_HPP_
#define SERVER_CLIENT_SOCKET_HPP_

#include "RequestParser/ServerRequestParser.hpp"
#include "ServerRequest.hpp"
#include "Socket.hpp"

class ConnectionState {
   public:
	enum State {
		IDLE,
		RECEIVING_REQUEST_HEADER_FROM_CLIENT,
		HANDLE_CGI_REQUEST,
		HANDLE_NORMAL_REQUEST,
		RESPONSE_SENT
	};

   private:
	ConnectionState();
};

struct ClientConnectionContext {
	t_request request;
	ServerRequestParser parser;
	ConnectionState::State state;
	bool cgi_started;

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
