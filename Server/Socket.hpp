#ifndef SERVER_SOCKET_HPP
#define SERVER_SOCKET_HPP

#include <string>
#include <netinet/in.h>
#include <status.hpp>

#define SOCKET_DEFAULT_MAX_CONNECTIONS 1024

class Socket {
public:
	Socket();
	Socket(Socket &other);
	virtual ~Socket();
	Socket &operator=(Socket &other);

	int get_fd() const;
	const struct sockaddr *get_address() const;
	socklen_t get_socklen() const;
	const std::string *get_host() const;
	int get_port() const;

	void set_socket(int socket, const struct sockaddr *sockaddr, socklen_t socklen);
	Status set_opt(int opt, bool to_set, int level = SOL_SOCKET);

	Status is_connected() const;
	Status close_socket();

private:
	Status set_host_ipv4_address_from_sockaddr();
	void set_port_ipv4_from_sockaddr();

protected:
	std::string _host;
	int _port;
	int _socket_fd;
	struct sockaddr _sockaddr;
	socklen_t _socklen;
};

#endif  // SERVER_SOCKET_HPP