#include "Server.hpp"

#include <errno.h>

#include <csignal>

#include "Exceptions/SystemException.hpp"
#include "IEventContext.hpp"
#include "IIOHandler.hpp"
#include "IOServerContext.hpp"
#include "IOServerHandler.hpp"
#include "ITimeoutTimer.hpp"
#include "ServerEventContext.hpp"
#include "ServerLogger.hpp"
#include "ServerSocket.hpp"
#include "colors.hpp"

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

	std::signal(SIGINT, sigint_handler);
	try {
		create_sockets_from_configs(_configs);
		while (g_signal_status != SIGINT) {
			_event.wait_event(0, &amount_of_events);
			if (amount_of_events > 0) {
				handle_epoll_event(amount_of_events);
			}
		}
	} catch (const std::exception& e) {
		_server_logger.log_error("Server::launch()", e.what());
		throw;
	}
	std::cout << "[Server] shutdown..." << std::endl;
}

void Server::handle_epoll_event(int amount_of_events) {
	std::map<int, IEventContext*> events_to_destroy;

	for (int i = 0; i < amount_of_events; ++i) {
		epoll_event& event = *_event[i];
		IEventContext& event_context = *static_cast<IEventContext*>(event.data.ptr);
		if (event.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
			events_to_destroy.insert(
				std::make_pair(event_context.get_fd()->get_fd(), &event_context));
			continue;
		}
		
		if (event.events & (EPOLLIN | EPOLLOUT | EPOLLHUP) &&
			event_context.get_io_handler()->is_closing() == false) {
			if (is_object_expired(event_context) == true) {
				event_context.get_timer()->stop();
			}

			event_context.get_io_handler()->handle(&event);
		}

		if (check_if_can_destroy_event(event.events, event_context, events_to_destroy) == true) {
			events_to_destroy.insert(
				std::make_pair(event_context.get_fd()->get_fd(), &event_context));
		}
	}

	destroy_events(events_to_destroy);
}

void Server::create_server_socket(const std::string& host, int port,
								  const t_config& server_config) {
	ServerSocket* server_socket = new ServerSocket(host, port, &server_config, &_server_logger);
	try {
		server_socket->start();
	} catch (const std::exception& e) {
		delete server_socket;
		throw;
	}

	IOServerContext* io_server_context = new IOServerContext;
	IOServerHandler* io_server_handler =
		new IOServerHandler(*server_socket, *io_server_context, _event, &_server_logger);
	ServerEventContext* server_event_context = new ServerEventContext();
	server_event_context->take_data_ownership(io_server_handler, io_server_context, server_socket,
											  NULL);

	_event.register_event(SERVER_EVENT_SERVER_EVENTS, server_socket->get_fd(),
						  server_event_context);
}

void Server::create_sockets_from_config(const t_config& server_config) {
	const size_t amount_of_addresses = server_config.listen.size();

	for (size_t i = 0; i < amount_of_addresses; ++i) {
		const std::string& host = server_config.listen[i].host;
		int port = server_config.listen[i].port;

		create_server_socket(host, port, server_config);
	}
}

void Server::create_sockets_from_configs(const std::vector<t_config>& configs) {
	for (size_t i = 0; i < configs.size(); ++i) {
		const t_config& config = configs[i];

		create_sockets_from_config(config);
	}
}

void Server::print_debug_addr(const std::string& address, int port) {
	std::cout << GREEN400 << "Listening at: " << address << ":" << port << RESET << std::endl;
}

bool Server::check_if_can_destroy_event(int events, IEventContext& event_context,
										std::map<int, IEventContext*>& events_to_destroy) {
	return is_object_expired(event_context) == true ||
		   (event_context.get_io_handler()->is_closing() == true &&
			events_to_destroy.find(event_context.get_fd()->get_fd()) == events_to_destroy.end());
}

bool Server::is_object_expired(IEventContext& event_context) {
	return (event_context.get_timer() != NULL && event_context.get_timer()->is_expired() == true);
}

void Server::destroy_events(std::map<int, IEventContext*>& events) {
	if (events.size() > 0) {
		std::map<int, IEventContext*>::iterator it = events.begin();
		while (it != events.end()) {
			std::cout << it->second->get_fd()->get_fd() << " closed connection" << std::endl;
			_event.unregister_event(it->second->get_fd()->get_fd());
			delete it->second;
			++it;
		}
	}
}