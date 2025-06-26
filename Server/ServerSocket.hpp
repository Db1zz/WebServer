#ifndef SERVER_SERVER_SOCKET_HPP
#define SERVER_SERVER_SOCKET_HPP

#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET_DEFAULT_MAX_CONNECTIONS 1024

class ServerSocket {
public:
	ServerSocket(std::string host, int port);
	ServerSocket(ServerSocket &other);
	~ServerSocket();
	ServerSocket &operator=(ServerSocket &other);
	/* getters */
	int get_fd() const;
	struct sockaddr_in get_address();

	/* setters */
	void set_socket(int socket);
	void set_address_conf(struct sockaddr_in address);
	void set_opt(int opt, bool to_set, int level = SOL_SOCKET);

	/* general functions */
	void close_socket();
	int start_connection();

protected:
	/* private functions */
	void is_socket_created(int socket_fd);
	void is_binded(int status);
	void is_listening(int status);

	/* private variables */
	std::string _host;
	int _port;
	int _socket_fd;
	struct sockaddr_in _address_conf;
	// const int _max_connections = 1024;
};

#endif  // SERVER_SERVER_SOCKET_HPP
