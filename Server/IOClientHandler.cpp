#include "IOClientHandler.hpp"

#include <sys/wait.h>

#include <cstdlib>

#include "CGIFileDescriptor.hpp"
#include "ClientSocket.hpp"
#include "FileDescriptor.hpp"
#include "HTTPResponseSender.hpp"
#include "IOCGIContext.hpp"
#include "IOCGIHandler.hpp"
#include "IOClientContext.hpp"
#include "ServerEvent.hpp"
#include "ServerLogger.hpp"
#include "ServerSocket.hpp"
#include "ServerSocketManager.hpp"
#include "ServerUtils.hpp"
#include "CGIEventContext.hpp"


IOClientHandler::IOClientHandler(ClientSocket& client_socket, IOClientContext& client_context,
								 ServerEvent& server_event, ServerLogger* server_logger)
	: _client_socket(client_socket),
	  _client_context(client_context),
	  _server_event(server_event),
	  _server_logger(server_logger),
	  _cgi_fd(-1),
	  _is_closing(false) {
}

IOClientHandler::~IOClientHandler() {
	CGIEventContext* cgi_event_context = static_cast<CGIEventContext*>(_server_event.get_event_context(_cgi_fd));
	if (cgi_event_context != NULL) {
		IOCGIHandler* io_cgi_handler = static_cast<IOCGIHandler*>(cgi_event_context->get_io_handler());
		io_cgi_handler->set_is_closing();
	}
}

Status IOClientHandler::read_and_parse() {
	Status status;

	if (_client_context.parser.is_finished() == false) {
		const ssize_t read_buff_size = 1000000;
		int rd_bytes = 0;

		status = server_utils::read_data(_client_socket.get_fd(), read_buff_size,
										 _client_context.buffer, rd_bytes);
		if (!status) {
			if (_server_logger != NULL) {
				_server_logger->log_error("IOClientHandler::read_and_parse", "failed to read data");
			}
			return status;
		}

		if (_client_context.parser.is_header_parsed() == false) {
			status =
				_client_context.parser.parse_header(_client_context.buffer, _client_context.buffer);
			if (!status) {
				if (_server_logger != NULL) {
					_server_logger->log_error("IOClientHandler::read_and_parse",
											  "failed to parse header");
				}
				return status;
			}
		}
		if (_client_context.parser.is_header_parsed() == true) {
			status = _client_context.parser.parse_body(_client_context.buffer);
			if (!status) {
				if (_server_logger != NULL) {
					_server_logger->log_error("IOClientHandler::read_and_parse",
											  "failed to parse body");
				}
				return status;
			}
		}
		_client_context.buffer.clear();
	}
	return status;
}

Status IOClientHandler::handle(void* data) {
	Status status;
	epoll_event& event = *static_cast<epoll_event*>(data);

	if (event.events & EPOLLIN) {
		status = read_and_parse();
		if (!status) {
			if (status != DataIsNotReady) {
				_server_logger->log_error("IOClientHandler::handle",
										  "failed with a error: '" + status.msg() + "'");
			}
			return status;
		}
	}
	if (event.events & EPOLLOUT) {
		if (_client_context.request.is_cgi == true) {
			return handle_cgi_request(status);
		}
		return handle_default_request(status);
	}
	return Status::OK();
}

bool IOClientHandler::is_closing() const {
	return _is_closing;
}

Status IOClientHandler::handle_cgi_request(Status status) {
	if (_client_context.cgi_pid < 0 && _client_context.request.is_request_ready()) {
		status = create_cgi_process();
		if (!status) {
			_server_logger->log_error("IOClientHandler::handle_cgi_request",
									  std::string("create_cgi_process failed with error: '") +
										  "TODO: Status error code!!!" + "'");
			return status;
		}
	} else if (_cgi_fd >= 0) {
		IEventContext* event_context = _server_event.get_event_context(_cgi_fd);
		IOCGIContext* io_cgi_context = static_cast<IOCGIContext*>(event_context->get_io_context());
		if (io_cgi_context->is_finished == false) {
			return Status::OK();
		}
		HTTPResponseSender response_sender(_client_socket, &io_cgi_context->request,
										   _client_context.server_config, _server_logger);
		status = response_sender.send();
		if (!status) {
			return status;
		}
		if (_server_logger != NULL) {
			_server_logger->log_access(_client_socket.get_host(), _client_context.request.method,
									   _client_context.request.uri_path,
									   _client_context.server_socket.get_port());
		}
		_client_context.reset();
	}
	return Status::OK();
}

Status IOClientHandler::handle_default_request(Status status) {
	if (status.code() != DataIsNotReady && _client_context.parser.is_header_parsed() == true) {
		HTTPResponseSender response_sender(_client_socket, &_client_context.request,
										   _client_context.server_config, _server_logger);
		status = response_sender.send();
		if (!status) {
			return Status(
				"HTTPResponseSender::send failed in IOClientHandler::handle_default_request " +
				status.msg());
		}
		if (_client_context.request.is_request_ready()) {
			if (_server_logger != NULL) {
				_server_logger->log_access(
					_client_socket.get_host(), _client_context.request.method,
					_client_context.request.uri_path, _client_context.server_socket.get_port());
			}
			_client_context.reset();
		}
	}
	return Status::OK();
}

Status IOClientHandler::create_cgi_process() {
	t_request& request = _client_context.request;
	pid_t cgi_process;
	int server_read_pipe[2];

	if (pipe(server_read_pipe) < 0) {
		_server_logger->log_error("IOClientHandler::create_cgi_process",
								  std::string("failed to create a pipe: ") + strerror(errno));
		return Status::InternalServerError();
	}

	cgi_process = fork();
	if (cgi_process < 0) {
		_server_logger->log_error(
			"IOClientHandler::create_cgi_process",
			std::string("failed to create a cgi process: ") + strerror(errno));
		close(server_read_pipe[0]);
		close(server_read_pipe[1]);
		return Status::InternalServerError();
	}

	if (cgi_process == 0) {
		std::string cgi_bin_filename;
		std::string script_path = "." + request.uri_path;
		server_utils::get_filename(request.cgi_bin, cgi_bin_filename);

		std::vector<std::string> argv_strs;
		argv_strs.push_back(cgi_bin_filename); // argv[0]
		argv_strs.push_back(script_path);	   // argv[1]
		std::vector<char*> argv;
		for (size_t i = 0; i < argv_strs.size(); ++i) {
			argv.push_back(const_cast<char*>(argv_strs[i].c_str()));
		}
		argv.push_back(NULL);

		std::vector<std::string> env_strs;
		{
			std::stringstream content_length;
			content_length << request.content_length;
			env_strs.push_back(std::string("REQUEST_METHOD=") + request.method);
			env_strs.push_back(std::string("CONTENT_LENGTH=") + content_length.str());
			env_strs.push_back(std::string("CONTENT_TYPE="));
			env_strs.push_back(std::string("SERVER_PROTOCOL=HTTP/1.1"));
			env_strs.push_back(std::string("SCRIPT_NAME="));
			env_strs.push_back(std::string("PATH_INFO="));
			env_strs.push_back(std::string("QUERY_STRING="));
			env_strs.push_back(std::string("REMOTE_ADDR="));
			env_strs.push_back(std::string("SERVER_NAME="));
			env_strs.push_back(std::string("SERVER_PORT="));
			env_strs.push_back(std::string("HTTP_USER_AGENT="));
			env_strs.push_back(std::string("HTTP_ACCEPT="));
			env_strs.push_back(std::string("SERVER_SOFTWARE=unravelThePuzzle"));
			env_strs.push_back(std::string("AUTH_TYPE=Basic"));
		}
		std::vector<char*> envp;
		for (size_t i = 0; i < env_strs.size(); ++i) {
			envp.push_back(const_cast<char*>(env_strs[i].c_str()));
		}
		envp.push_back(NULL);

		if (dup2(server_read_pipe[1], STDOUT_FILENO) < 0) {
			_server_logger->log_error(
				"IOClientHandler::create_cgi_process",
				std::string("dup2() failed with a error: ") + strerror(errno));
			std::exit(127);
		}

		close(server_read_pipe[0]);
		close(server_read_pipe[1]);

		execve(request.cgi_bin.c_str(), argv.data(), envp.data());

		_server_logger->log_error("IOClientHandler::create_cgi_process",
								  std::string("execve() failed with a error: ") + strerror(errno));
		std::exit(127);
	}

	close(server_read_pipe[1]);

	CGIFileDescriptor* cgi_fd = new CGIFileDescriptor(server_read_pipe[0], _client_socket);
	IOCGIContext* cgi_context =
		new IOCGIContext(*cgi_fd, _client_context.server_config, _server_logger);
	IOCGIHandler* cgi_handler = new IOCGIHandler(*cgi_fd, *cgi_context, _client_context,
												 _client_context.server_config, _server_logger);
	CGIEventContext* cgi_event_context = new CGIEventContext();
	cgi_event_context->take_data_ownership(cgi_handler, cgi_context, cgi_fd);

	_server_event.register_event(SERVER_EVENT_CLIENT_EVENTS, cgi_fd->get_fd(), cgi_event_context);
	_client_context.cgi_fd = cgi_fd->get_fd();
	_client_context.cgi_pid = cgi_process;
	_cgi_fd = cgi_fd->get_fd();
	return Status::OK();
}
