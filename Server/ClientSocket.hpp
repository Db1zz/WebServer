#ifndef SERVER_CLIENT_SOCKET_HPP_
#define SERVER_CLIENT_SOCKET_HPP_

#include "Socket.hpp"

class ClientSocket : public Socket {
   public:
	ClientSocket();
	~ClientSocket();

	void set_server_fd(int server_fd);
	int get_server_fd();

	std::string& get_request_buffer();

	void reset_request_buffer();
	bool is_request_ready();
	void set_request_ready();

   private:
	std::string _request_buffer;
	bool _request_ready;
	int _server_fd;
};

#endif // SERVER_CLIENT_SOCKET_HPP_