#include "IOCGIHandler.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include "CGIFileDescriptor.hpp"
#include "CGIResponseParser.hpp"
#include "HTTPResponseSender.hpp"
#include "IOCGIContext.hpp"
#include "IOClientContext.hpp"
#include "ServerEvent.hpp"

IOCGIHandler::IOCGIHandler(CGIFileDescriptor& cgi_fd, IOCGIContext& io_cgi_context,
						   IOClientContext& io_client_context, const t_config* server_config,
						   ServerLogger* server_logger)
	: _cgi_fd(cgi_fd),
	  _io_cgi_context(io_cgi_context),
	  _io_client_context(io_client_context),
	  _server_config(server_config),
	  _server_logger(server_logger),
	  _child_last_msg_time(timer::now()),
	  _is_closing(false) {
}

IOCGIHandler::~IOCGIHandler() {
}

Status IOCGIHandler::handle(void* data) {
	Status status;
	size_t buffer_size = 1000000;
	char buffer[buffer_size];
	bool is_timeouted = false;

	ssize_t rd_bytes = read(_cgi_fd.get_fd(), buffer, buffer_size);
	if (rd_bytes < 0) {
		_server_logger->log_error("Server::cgi_fd_routine", "failed to read data from a cgi pipe");
		return Status::InternalServerError();
	} else {
		_child_last_msg_time = timer::now();
	}

	std::string content;
	int wpidstatus = 0;

	waitpid(_io_cgi_context.cgi_pid, &wpidstatus, 0);
	// if (timer::diff(_child_last_msg_time, timer::now()) > IO_HANDLER_TIMEOUT) {
	// 	content.append(
	// 		"Status: 504 Gateway Timeout\r\n"
	// 		"Content-Type: text/plain\r\n"
	// 		"\r\n"
	// 		"An timeout occured during CGI exectuion\r\n");
	// 	is_timeouted = true;
	if ((WIFSIGNALED(wpidstatus) || WIFEXITED(wpidstatus)) && WEXITSTATUS(wpidstatus) != 0) {
		//std::cout << WTERMSIG(wpidstatus) << std::endl;
		content.append(
			"Status: 500 Internal Server Error\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n"
			"An internal error occurred. Please try again later.\r\n");
	} else {
		if (rd_bytes > 0) {
			content.append(buffer, rd_bytes);
		}
	}
	status = _io_cgi_context.cgi_parser.parse(content);
	if (!status) {
		_server_logger->log_error("Server::cgi_fd_routine", "failed to parse CGI response");
		return status;
	}
	if (status != DataIsNotReady) {
		_io_cgi_context.is_finished = true;
	}

	// not sure about this btw
	// if (is_timeouted == true) {
	// 	return Status::GatewayTimeout();
	// }

	return status;
}

bool IOCGIHandler::is_closing() const {
	return _is_closing;
}

void IOCGIHandler::set_is_closing() {
	_is_closing = true;
}