#ifndef SERVER_CLIENT_SOCKET_HPP_
#define SERVER_CLIENT_SOCKET_HPP_

#include "Socket.hpp"

#define REQUEST_BUFFER_IS_EMPTY 0

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
	size_t get_request_content_length();
	void set_request_content_length(size_t request_content_length);

   private:
	size_t _request_content_length;
	std::string _request_buffer;
	bool _request_ready;
	int _server_fd;
};

#endif // SERVER_CLIENT_SOCKET_HPP_

// #ifndef SERVER_CLIENT_SOCKET_HPP_
// #define SERVER_CLIENT_SOCKET_HPP_

// #include "Socket.hpp"
// #include "ServerRequest.hpp"

// class ClientSocket : public Socket {
//    public:
// 	ClientSocket();
// 	~ClientSocket();

// 	void set_server_fd(int server_fd);
// 	int get_server_fd();

// 	t_request *get_request();
// 	void reset_request();

//    private:
// 	t_request *_request;
// 	int _server_fd;
// };

// #endif // SERVER_CLIENT_SOCKET_HPP_