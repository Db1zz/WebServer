#include "Server.hpp"

#include <errno.h>
#include <netdb.h> /* getnameinfo() */
#include <string.h>

#include <cstdio>

Server::Server(std::vector<t_config> configs)
	: _configs(configs)
{
	init();
}

Server::~Server() {
	destroy_sockets();
}

void Server::destroy_sockets() {
	for (size_t i = 0; i < _sockets.size(); ++i) {
		if (_sockets[i]) {
			delete _sockets[i];
		}
	}
}

void Server::launch() {
	int amount_of_events = 0;

	while (true) {
		amount_of_events = _event.wait_event(-1);
		if (amount_of_events > 0) {
			handle_event(amount_of_events);
		}
	}
}

void Server::init() {
	create_sockets_from_configs();
}

/*
	Notes:
		1. request_event.data.fd == _sockets[j]->get_fd() means that there's a new signal
*/
void Server::handle_event(int amount_of_events) {
	for (int i = 0; i < amount_of_events; ++i) {
		const epoll_event &request_event = *_event[i];
		for (size_t j = 0; j < _sockets.size(); ++j) {
			if (request_event.data.fd == _sockets[j]->get_fd()) {
				accept_new_connection(request_event.data.fd);
			} else {
				// handle request and generate response
				std::vector<std::string> request;
				
				std::cout << "EPOLLIN: " << (request_event.events & EPOLLIN) << std::endl;
				std::cout << "EPOLLOUT: " << (request_event.events & EPOLLOUT) << std::endl;
				if (request_event.events & EPOLLIN) {
					request = request_handler(request_event);
					request.size(); // suppress unused variable err
				} else if (request_event.events & EPOLLOUT) {
					t_request req;
					req.method = "GET";
					req.uri_path = "/";
					req.user_agent = "";
					req.host = "localhost";
					req.language = "";
					req.connection = "keep-alive";
					req.mime_type = "html";
					req.content_type = "text/html";
	
					response_handler(request_event, req);
					close(request_event.data.fd);
				}
			}
		}
	}
}

std::vector<std::string> Server::read_request(const epoll_event &request_event) {
	const size_t read_buff_size = 10024;
	char read_buff[read_buff_size];	 // TODO: read about what size of the header
	ssize_t rd_bytes = 0;
	/*
		Checking the value of errno is strictly forbidden after performing a read or write operation. :(
	*/
	rd_bytes = read(request_event.data.fd, read_buff, read_buff_size);

	/*
		This block of code will be commented out due to the non-blocking socket behavior.
		This may cause random characters to appear in read_buff:
		https://www.scottklement.com/rpg/socktut/nonblocking.html
	*/
	// if (rd_bytes < 0 && ) {

	// } eles {
	// 	close(request_event.data.fd);
	// 	throw std::runtime_error("read() failed!");
	// }
	read_buff[rd_bytes] = 0;
	std::cout << CYAN300 << "REQUEST:\n" << read_buff << RESET << std::endl;
	return std::vector<std::string>();	// return empty arr
}

std::vector<std::string> Server::request_handler(
	const epoll_event &request_event) {
	std::vector<std::string> request = read_request(request_event);

	/*
		Goshan41k

		We can validate request during parsing or before/after it,
		depends on our implementation.

		Parser should return struct, which will be used later in
	   response_handler(), right now I'm returning std::vector<std::string> cuz
	   nothing is implemented and i'm not sure how this struct will look like.
	*/
	// parse_request()
	// request_validator();
	return request;
}

void Server::response_handler(const epoll_event &request_event,
							  const t_request &request) {
	ServerResponse resp(request, _configs[0]);
	std::string res = resp.generate_response();
	if (write(request_event.data.fd, res.c_str(), res.size()) < 0) {
		perror("write");
	}
}

std::string Server::response_generator(/* TODO: add args*/) {
	std::string response_str;
	response_str = "some response";
	return response_str;
}

void Server::accept_new_connection(int socket_fd) {
	struct sockaddr cl_sockaddr;
	socklen_t cl_len = sizeof(cl_sockaddr);
	int cl_fd;

	cl_fd = accept(socket_fd, &cl_sockaddr, &cl_len);
	if (cl_fd < 0) {
		std::runtime_error("accept() failed: " + std::string(strerror(errno)));
	}

	// fcntl(cl_fd, F_SETFL, O_NONBLOCK);
	_event.add_event(EPOLLIN | EPOLLOUT, cl_fd);
	announce_new_connection(cl_sockaddr, cl_fd);
}

void Server::announce_new_connection(const struct sockaddr &cl_sockaddr,
									 int cl_fd) {
	char hbuff[NI_MAXHOST];
	char sbuff[NI_MAXSERV];
	int status = 0;

	status =
		getnameinfo(&cl_sockaddr, sizeof(cl_sockaddr), sbuff, sizeof(sbuff),
					hbuff, sizeof(hbuff), NI_NUMERICHOST | NI_NUMERICSERV);
	if (status != 0) {
		std::runtime_error("genameinfo() failed");
	}
	printf(
		"Accepted new connection on descriptor %d\n"
		"(host: %s, addr: %s)\n",
		cl_fd, hbuff, sbuff);
}

void Server::set_default_host_and_port_if_needed(t_config &config)
{
	if (config.host.size() == 0) {
		config.host.push_back(SERVER_DEFAULT_ADDR);
	}
	if (config.port.size() == 0) {
		config.port.push_back(SERVER_DEFAULT_PORT);
	}
}

void Server::create_sockets_from_configs() {
	int yes = 1;

	_sockets.reserve(_configs.size());
	for (size_t i = 0; i < _configs.size(); ++i) {
		t_config &config = _configs[i];
		set_default_host_and_port_if_needed(config);
		for (size_t j = 0; j < config.host.size(); ++j) {
			for (size_t k = 0; k < config.port.size(); ++k) {
				ServerSocket* new_socket = new ServerSocket(config.host[j], atoi(config.port[k].c_str()));
				int socket_fd = new_socket->get_fd();

				print_debug_addr(config.host[j], config.port[k]); // REMOVEME

				setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
				new_socket->start_connection();
				
				_event.add_event(EPOLLIN | EPOLLOUT, socket_fd);
				_sockets.push_back(new_socket);
			}
		}
	}
}

void Server::print_debug_addr(const std::string &address, const std::string &port) {
	std::cout << GREEN400 << "Listening at: " << address << ":" << port << RESET << std::endl;
}