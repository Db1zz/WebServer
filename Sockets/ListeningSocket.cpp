#include "ListeningSocket.hpp"

ListeningSocket::ListeningSocket(int domain, int service, int protocol, int port, u_long interface, int backlog): ServerSocket(domain, service, protocol, port, interface), _backlog(backlog) {
	start_listening();
	test_connection(_listening);
}

void ListeningSocket::start_listening() {
	_listening = listen(get_socket(), _backlog);
}