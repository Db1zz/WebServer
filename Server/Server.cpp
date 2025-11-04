#include "Server.hpp"

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cassert>
#include <csignal>
#include <cstdio>
#include <sstream>

#include "ClientSocket.hpp"
#include "IIOHandler.hpp"
#include "IOServerContext.hpp"
#include "IOServerHandler.hpp"
#include "RequestParser/ServerRequestParser.hpp"
#include "ServerConfig.hpp"
#include "ServerLogger.hpp"
#include "ServerRequest.hpp"
#include "ServerResponse.hpp"
#include "ServerSocket.hpp"
#include "ServerSocketManager.hpp"
#include "ServerUtils.hpp"
#include "Socket.hpp"
#include "status.hpp"
#include "ServerEventContext.hpp"


namespace {
volatile std::sig_atomic_t g_signal_status = 0;
}

void sigint_handler(int signal) {
	g_signal_status = signal;
}

Server::Server(const std::vector<t_config>& configs, ServerLogger& server_logger)
	: _configs(configs), _server_logger(server_logger) {
}

Server::~Server() {
}

Status Server::launch() {
	Status status;
	int amount_of_events = 0;

	status = create_sockets_from_configs(_configs);
	if (!status) {
		_server_logger.log_error("Server::launch()", status.msg());
		return status;
	}

	while (g_signal_status != SIGINT) {
		status = _event.wait_event(0, &amount_of_events);
		if (!status && status.error() != EINTR) {
			return Status("Server::launch() failed with a error: '" + status.msg() + "'");
		}

		if (amount_of_events > 0) {
			status = handle_epoll_event(amount_of_events);
			if (!status) {
				return Status("Server::launch() failed with a error: '" + status.msg() + "'");
			}
		}
	}
	std::cout << "[Server] shutdown..." << std::endl;
	return Status::OK();
}

Status Server::handle_epoll_event(int amount_of_events) {
	Status status;

	for (int i = 0; i < amount_of_events; ++i) {
		epoll_event& event = *_event[i];
		IEventContext& event_context = *static_cast<IEventContext*>(event.data.ptr);
		if (event.events & (EPOLLERR | EPOLLRDHUP) || event_context.get_io_handler()->is_closing() == true) {
			_event.unregister_event(event_context.get_fd()->get_fd());
			std::cout << "closed connection\n";
		} else if (event.events & (EPOLLIN | EPOLLOUT)) {
			status = event_context.get_io_handler()->handle(&event);
			if (!status) {
				_server_logger.log_error("Server::handle_event",
										 "failed to handle an event: '" + status.msg() + "'");
				return status;
			}
		}
	}
	return Status::OK();
}

Status Server::create_server_socket_manager(const std::string& host, int port,
											const t_config& server_config) {
	Status status;

	ServerSocketManager* server_socket_manager = new ServerSocketManager(host, port, &_event, server_config, &_server_logger);
	status = server_socket_manager->start();
	if (!status) {
		delete server_socket_manager;
		return status;
	}

	IOServerContext* io_server_context = new IOServerContext;
	IOServerHandler* io_server_handler = new IOServerHandler(*server_socket_manager->get_server_socket(), *io_server_context, &_server_logger);
	ServerEventContext* server_event_context = new ServerEventContext();
	server_event_context->take_data_ownership(io_server_handler, io_server_context, server_socket_manager->get_server_socket());

	io_server_context->server_socket_manager = server_socket_manager;

	status = _event.register_event(SERVER_EVENT_SERVER_EVENTS, server_socket_manager->get_server_socket()->get_fd(), server_event_context);
	return status;
}

Status Server::create_sockets_from_config(const t_config& server_config) {
	const size_t amount_of_addresses = server_config.listen.size();
	Status status;

	for (size_t i = 0; i < amount_of_addresses; ++i) {
		const std::string& host = server_config.listen[i].host;
		int port = server_config.listen[i].port;

		status = create_server_socket_manager(host, port, server_config);
		if (!status) {
			return status;
		}
	}
	return Status::OK();
}

Status Server::create_sockets_from_configs(const std::vector<t_config>& configs) {
	Status status;

	for (size_t i = 0; i < configs.size(); ++i) {
		const t_config& config = configs[i];

		status = create_sockets_from_config(config);
		if (!status) {
			return status;
		}
	}
	return Status::OK();
}

// void Server::destroy_all_server_socket_managers() {
// 	std::map<int, EventContext*>::iterator it;

// 	it = _events_contexts.begin();
// 	while (it != _events_contexts.end()) {
// 		IOServerContext* server_context = static_cast<IOServerContext*>(it->second->context);
// 		_event.remove_event(server_context->server_socket_manager->get_server_socket()->get_fd());
// 		delete server_context->server_socket_manager;
// 		delete server_context;
// 		delete it->second->handler;
// 		++it;
// 	}
// 	_events_contexts.clear();
// }

void Server::print_debug_addr(const std::string& address, int port) {
	std::cout << GREEN400 << "Listening at: " << address << ":" << port << RESET << std::endl;
}
