#ifndef SERVER_CLIENT_SOCKET_HPP_
#define SERVER_CLIENT_SOCKET_HPP_

#include "Socket.hpp"
#include "ServerRequest.hpp"

class ClientSocket : public Socket {
   public:
	ClientSocket();
	~ClientSocket();

	void set_server_fd(int server_fd);
	int get_server_fd();

	void reset_request();
	t_request* get_request_data();

   private:
	t_request* _request_data;
	int _server_fd;
};

#endif // SERVER_CLIENT_SOCKET_HPP_
