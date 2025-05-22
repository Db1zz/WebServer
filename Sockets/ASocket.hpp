#ifndef ASOCKET_HPP
#define ASOCKET_HPP
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include "colors.hpp"

class ASocket {
private:
	int _socket;
	int _connection;
	struct sockaddr_in _address;

	ASocket(const ASocket &other);
	ASocket &operator= (const ASocket &other);
public:
	ASocket(int domain, int service, int protocol, int port, u_long interface);
	~ASocket();
	/*getters*/
	int get_socket() const;
	int get_connection() const;
	struct sockaddr_in get_address();

	/*setters*/
	void set_socket(int socket);
	void set_connection(int connection);
	void set_address(struct sockaddr_in address);

	virtual int start_connection(int socket, struct sockaddr_in address) = 0;
	void test_connection(int connection);

	/*exceptions*/
	class CannotConnectException: public std::exception {
		public:
			const char* what() const throw() { return "failed to connect."; }
	};

	//TODO:
	//add listening
	//add accepting connections
	//add closing
	//add error messages -> directly to socket or error class in general?
	//exceptions are on different class? create a task to implement either an error class or exceptions class?
	//look up -> blocking, non-blocking and asynchronous socket calls

	//either remove connection or use it with setter in derived class???

};

#endif
