#ifndef SERVER_CLIENT_SOCKET_HPP_
#define SERVER_CLIENT_SOCKET_HPP_

#include "RequestParser/ServerRequestParser.hpp"
#include "ServerRequest.hpp"
#include "Socket.hpp"
#include <map>

class FileDescriptor;

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
	std::string buffer;
	bool cgi_started;

	std::map<int, FileDescriptor *> descriptors;

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
	FileDescriptor _server_fd;
};

#endif // SERVER_CLIENT_SOCKET_HPP_
