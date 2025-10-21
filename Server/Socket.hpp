#ifndef SERVER_SOCKET_HPP
#define SERVER_SOCKET_HPP

#include <string>
#include <netinet/in.h>
#include <status.hpp>

#include "FileDescriptor.hpp"

#define SOCKET_DEFAULT_MAX_CONNECTIONS 100024

class Socket : public FileDescriptor {
public:
	enum SocketOption {
		kReuseAddr
		// KReusePort
	};

	enum SetMode {
		kUnset,
		kSet
	};

	enum SocketType {
		STANDARD_SOCKET,
		CLIENT_SOCKET,
		SERVER_SOCKET,
	};

	Socket();
	Socket(std::time_t idle_time);
	Socket(Socket &other);
	virtual ~Socket();
	Socket &operator=(Socket &other);

	const struct sockaddr *get_address() const;
	socklen_t get_socklen() const;
	const std::string &get_host() const;
	int get_port() const;

	void set_socket(int socket, const struct sockaddr *sockaddr, socklen_t socklen);
	Status set_socket_option(SocketOption socket_option, SetMode mode);

	Status is_connected() const;
	void close_socket();
	SocketType get_socket_type() const;

private:
	Status set_host_ipv4_address_from_sockaddr();
	void set_port_ipv4_from_sockaddr();

protected:
	SocketType _socket_type;
	std::string _host;
	int _port;
	struct sockaddr _sockaddr;
	socklen_t _socklen;
};

#endif  // SERVER_SOCKET_HPP