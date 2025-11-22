#include "ServerRequestParser.hpp"

#include "RequestMultipartBodyParser.hpp"
#include "RequestRawBodyParser.hpp"
#include "ServerRequest.hpp"
#include "ServerConfig.hpp"
#include "status.hpp"

ServerRequestParser::ServerRequestParser(const t_config* config, ServerLogger* logger)
	: 
	  _config(config),
	  _logger(logger),
	  _header_parser(config, logger),
	  _body_parser(NULL),
	  _header_parsed(false),
	  _body_parsed(false) {
}

ServerRequestParser::~ServerRequestParser() {
	delete _body_parser;
}

Status ServerRequestParser::parse(const std::string& content, t_request& request) {
	Status status;
	size_t body_pos_start = 0;

	if (_header_parsed == false) {
		status = parse_header(content, body_pos_start, request);
		if (!status) {
			if (_logger != NULL) {
				_logger->log_error("ServerRequestParser::parse()", "failed to parse header: " + status.msg());
			}
			return status;
		}
	}
	if (_header_parsed == true && _body_parser != NULL) {
		status = parse_body(content, body_pos_start, request);
		if (!status) {
			if (_logger != NULL) {
				_logger->log_error("ServerRequestParser::parse()", "failed to parse body: " + status.msg());
			}
			return status;
		}
	}

	return status;
}

Status ServerRequestParser::parse_header(const std::string& content, size_t& body_pos_start, t_request& request) {
	if (_header_parsed == true) {
		return Status::OK();
	}

	Status status;

	status = _header_parser.feed(content, body_pos_start);
	if (!status) {
		return status;
	}

	status = _header_parser.apply(request);
	if (!status || status.error() == DataIsNotReady) {
		return status;
	}

	if (request.method == "POST" || request.is_cgi == true) {
		create_body_parser(request);
	}

	_header_parsed = true;

	return status;
}

Status ServerRequestParser::parse_body(const std::string& content, size_t body_pos_start, t_request& request) {
	Status status;

	status = _body_parser->feed(content, body_pos_start);
	if (!status) {
		return status;
	}

	if (status != DataIsNotReady) {
		_body_parser->apply(request);
	}

	if (is_body_parsed() == true) {
		_body_parsed = true;
	}

	if (_body_parsed == true && request.transfered_length != request.content_length) {
		return Status::BadRequest();
	}

	return status;
}

void ServerRequestParser::reset() {
	_header_parser = RequestHeaderParser(_config, _logger);
	_header_parsed = false;
	_body_parsed = false;
	delete _body_parser;
	_body_parser = NULL;
}

bool ServerRequestParser::is_header_parsed() const {
	return _header_parsed;
}

bool ServerRequestParser::is_body_parsed() const {
	return (_body_parser == NULL) || _body_parser->is_finished();
}

bool ServerRequestParser::is_finished() const {
	return is_header_parsed() && is_body_parsed();
}

void ServerRequestParser::create_body_parser(t_request& request) {
	if (request.is_cgi) {
		_body_parser = new RequestRawBodyParser(request.content_length, InFile);
	} else if (request.content_type.type == "multipart" &&
			   request.content_type.subtype == "form-data") {
		const std::string* boundary = request.content_type.find_parameter("boundary");
		_body_parser = new RequestMultipartParser(*boundary, request.content_length, _logger);
	} else {
		_body_parser = new RequestRawBodyParser(request.content_length, InBuffer);
	}
}