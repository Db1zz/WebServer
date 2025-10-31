#include "IOCGIHandler.hpp"

#include <unistd.h>

#include "CGIFileDescriptor.hpp"
#include "CGIResponseParser.hpp"
#include "HTTPResponseSender.hpp"
#include "IOCGIContext.hpp"
#include "IOClientContext.hpp"
#include "ServerEvent.hpp"

IOCGIHandler::IOCGIHandler(CGIFileDescriptor& cgi_fd, IOCGIContext& io_cgi_context, IOClientContext& io_client_context,
						   const t_config* server_config, ServerLogger* server_logger)
	: _cgi_fd(cgi_fd),
	  _io_cgi_context(io_cgi_context),
	  _io_client_context(io_client_context),
	  _server_config(server_config),
	  _server_logger(server_logger) {
}

IOCGIHandler::~IOCGIHandler() {
}

Status IOCGIHandler::handle(void* data) {
	Status status;
	size_t buffer_size = 1000000;
	char buffer[buffer_size];

	ssize_t rd_bytes = read(_cgi_fd.get_fd(), buffer, buffer_size);
	if (rd_bytes == 0) {
		_cgi_fd.close_fd();
		return Status::OK();
	}

	if (rd_bytes < 0) {
		_server_logger->log_error("Server::cgi_fd_routine", "failed to read data from a cgi pipe");
		return Status::InternalServerError();
	}

	std::string content;
	content.append(buffer, rd_bytes);

	status = _io_cgi_context.cgi_parser.parse(content);
	if (!status) {
		_server_logger->log_error("Server::cgi_fd_routine", "failed to parse CGI response");
		return status;
	}

	if (status != DataIsNotReady) {
		_io_cgi_context.is_finished = true;
	}

	return status;
}
