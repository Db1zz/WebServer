#ifndef SOCKETS_ASOCKET_HPP
#define SOCKETS_ASOCKET_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

class ASocket {
public:
	ASocket(int domain, int service, int protocol, int port, u_long interface);
	virtual ~ASocket();

	/* getters */
	int get_fd() const;
	struct sockaddr_in get_address();

	/* setters */
	void set_socket(int socket);
	void set_address(struct sockaddr_in address);
	void set_opt(int opt, bool to_set, int level = SOL_SOCKET);

	/* general functions */
	virtual int start_connection() = 0;
	void close_socket();

	//TODO:
	//add listening
	//add accepting connections
	//add error messages -> directly to socket or error class in general?
	//exceptions are on different class? create a task to implement either an error class or exceptions class?
	//look up -> blocking, non-blocking and asynchronous socket calls

	//either remove connection or use it with setter in derived class???
	
protected:
	/* deleted operations */
	ASocket(const ASocket &other);
	ASocket &operator= (const ASocket &other);

	/* private functions */
	void is_socket_created(int socket_fd);

	/* private variables */
	int _socket_fd;
	struct sockaddr_in _address;
};

#endif  // SOCKET_ASOCKET_HPP
