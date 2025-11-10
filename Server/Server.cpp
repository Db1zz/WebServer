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
#include "ITimeoutTimer.hpp"
#include "RequestParser/ServerRequestParser.hpp"
#include "ServerConfig.hpp"
#include "ServerEventContext.hpp"
#include "ServerLogger.hpp"
#include "ServerRequest.hpp"
#include "ServerResponse.hpp"
#include "ServerSocket.hpp"
#include "ServerSocketManager.hpp"
#include "ServerUtils.hpp"
#include "Socket.hpp"
#include "status.hpp"

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

void Server::launch() {
	int amount_of_events = 0;

	try {
		create_sockets_from_configs(_configs);
		while (g_signal_status != SIGINT) {
			_event.wait_event(0, &amount_of_events);
			if (amount_of_events > 0) {
				handle_epoll_event(amount_of_events);
			}

			// if (!status && status.error() != EINTR) {
			// 	return Status("Server::launch() failed with a error: '" + status.msg() + "'");
			// }
		}
	} catch (const std::exception& e) {
		_server_logger.log_error("Server::launch()", e.what());
	}
	std::cout << "[Server] shutdown..." << std::endl;
}

void Server::handle_epoll_event(int amount_of_events) {
	std::map<int, IEventContext*> events_to_destroy;

	for (int i = 0; i < amount_of_events; ++i) {
		epoll_event& event = *_event[i];
		IEventContext& event_context = *static_cast<IEventContext*>(event.data.ptr);
		if (event.events & EPOLLERR ||
			(event_context.get_io_handler()->is_closing() == true &&
			 events_to_destroy.find(event_context.get_fd()->get_fd()) != events_to_destroy.end())) {
			events_to_destroy.insert(
				std::make_pair(event_context.get_fd()->get_fd(), &event_context));
		} else if (event.events & (EPOLLIN | EPOLLOUT | EPOLLHUP) &&
				   event_context.get_io_handler()->is_closing() == false) {
			if (event_context.get_timer() != NULL &&
				event_context.get_timer()->is_expired() == true) {
				event_context.get_timer()->stop();
				events_to_destroy.insert(
					std::make_pair(event_context.get_fd()->get_fd(), &event_context));
			}

			event_context.get_io_handler()->handle(&event);
			// if (!status) {
			// 	_server_logger.log_error("Server::handle_event",
			// 							 "failed to handle an event: '" + status.msg() + "'");
			// 	return status;
			// }
		}
	}

	if (events_to_destroy.size() > 0) {
		std::map<int, IEventContext*>::iterator it = events_to_destroy.begin();
		while (it != events_to_destroy.end()) {
			std::cout << it->second->get_fd()->get_fd() << " closed connection" << std::endl;
			_event.unregister_event(it->second->get_fd()->get_fd());
			delete it->second;

			++it;
		}
	}
}

void Server::create_server_socket_manager(const std::string& host, int port,
										  const t_config& server_config) {
	ServerSocketManager* server_socket_manager =
		new ServerSocketManager(host, port, &_event, server_config, &_server_logger);

	try {
		server_socket_manager->start();
	} catch (const std::exception& e) {
		delete server_socket_manager;
	}

	IOServerContext* io_server_context = new IOServerContext;
	IOServerHandler* io_server_handler = new IOServerHandler(
		*server_socket_manager->get_server_socket(), *io_server_context, &_server_logger);
	ServerEventContext* server_event_context = new ServerEventContext();
	server_event_context->take_data_ownership(io_server_handler, io_server_context,
											  server_socket_manager->get_server_socket(), NULL);

	io_server_context->server_socket_manager = server_socket_manager;

	_event.register_event(SERVER_EVENT_SERVER_EVENTS,
						  server_socket_manager->get_server_socket()->get_fd(),
						  server_event_context);
}

void Server::create_sockets_from_config(const t_config& server_config) {
	const size_t amount_of_addresses = server_config.listen.size();

	for (size_t i = 0; i < amount_of_addresses; ++i) {
		const std::string& host = server_config.listen[i].host;
		int port = server_config.listen[i].port;

		create_server_socket_manager(host, port, server_config);
	}
}

void Server::create_sockets_from_configs(const std::vector<t_config>& configs) {
	for (size_t i = 0; i < configs.size(); ++i) {
		const t_config& config = configs[i];

		create_sockets_from_config(config);
	}
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
