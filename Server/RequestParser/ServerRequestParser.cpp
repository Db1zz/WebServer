#include "ServerRequestParser.hpp"

#include "RequestMultipartBodyParser.hpp"
#include "RequestRawBodyParser.hpp"
#include "ServerRequest.hpp"
#include "status.hpp"

ServerRequestParser::ServerRequestParser(t_request* request, ServerLogger* logger)
	: _logger(logger), _header_parser(logger), _body_parser(NULL), _header_found(false), _request(request) {
}

ServerRequestParser::~ServerRequestParser() {
	if (_body_parser) {
		delete _body_parser;
		_body_parser = NULL;
	}
}

void ServerRequestParser::create_body_parser() {
	if (_request->content_type.type == "multipart" && _request->content_type.subtype == "form-data") {
		const std::string* boundary = _request->content_type.find_parameter("boundary");
		_body_parser = new RequestMultipartParser(*boundary, _request->content_length);
	} else {
		_body_parser = new RequestRawBodyParser(_request->content_length);
	}
}

Status ServerRequestParser::feed(const std::string& content) {
	Status status;
	size_t cursor_pos = 0;

	if (!_header_found) {
		status = _header_parser.feed(content, cursor_pos);
		if (!status) {
			return status;
		}
		status = _header_parser.apply(*_request);
		if (!status || status.error() == DataIsNotReady) {
			return status;
		}
		_header_found = true;

		if (_request->method == "POST") {
			create_body_parser();
		}
	}

	if (_request->method == "POST") {
		status = _body_parser->feed(content, cursor_pos);
		if (!status) {
			return status;
		}
		if (status != DataIsNotReady) {
			_body_parser->apply(*_request);
		}
	}

	return Status::OK();
}