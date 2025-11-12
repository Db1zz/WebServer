#include "IOCGIHandler.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>

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
	  _timeout_timer(NULL),
	  _is_closing(false) {
}

IOCGIHandler::~IOCGIHandler() {
}

void IOCGIHandler::handle(void* data) {
	std::string content;

	(void) data;
	if (_timeout_timer != NULL && _timeout_timer->is_expired() == true) {
		handle_timeout(content);
	} else if (handle_default(content) == false) {
		_io_client_context.is_cgi_request_finished = true;
		return;
	}

	_status = _io_cgi_context.cgi_parser.parse(content);
	if (!_status) {
		_server_logger->log_error("IOCGIHandler::handle", "failed to parse CGI response");
	}

	if (_status != DataIsNotReady) {
		try {
			_io_client_context.is_cgi_request_finished = true;
			HTTPResponseSender response_sender(_io_client_context.client_socket,
											   &_io_cgi_context.request, _server_config,
											   _io_client_context.server_socket, _server_logger);
			response_sender.send(_status);
			set_is_closing();
		} catch (const std::exception& e) {
			if (_server_logger != NULL) {
				_server_logger->log_error("IOCGIHandler::handle",
										  "failed to send response: " + std::string(e.what()));
			}
			throw;
		}
	}
}

bool IOCGIHandler::is_closing() const {
	return _is_closing;
}

void IOCGIHandler::set_is_closing() {
	_is_closing = true;
}

void IOCGIHandler::handle_timeout(std::string& result) {
	result.append(
		"Status: 504 Gateway Timeout\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"An timeout occured during CGI exectuion\r\n");
	_io_client_context.is_cgi_request_finished = true;
	_status = Status::GatewayTimeout();
}

bool IOCGIHandler::handle_default(std::string& result) {
	size_t buffer_size = 1000000;
	char buffer[buffer_size];

	ssize_t rd_bytes = read(_cgi_fd.get_fd(), buffer, buffer_size);
	if (rd_bytes < 0) {
		return false;
	}

	int wpidstatus = 0;
	waitpid(_io_cgi_context.cgi_pid, &wpidstatus, 0);
	if ((WIFSIGNALED(wpidstatus) || WIFEXITED(wpidstatus)) && WEXITSTATUS(wpidstatus) != 0) {
		result.append(
			"Status: 500 Internal Server Error\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n"
			"An internal error occurred. Please try again later.\r\n");
		_status = Status::InternalServerError();
		return true;
	}

	if (rd_bytes > 0) {
		result.append(buffer, rd_bytes);
	}
	return true;
}

void IOCGIHandler::set_timeout_timer(ITimeoutTimer* timeout_timer) {
	_timeout_timer = timeout_timer;
}
