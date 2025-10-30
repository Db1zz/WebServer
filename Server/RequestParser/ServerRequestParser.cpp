#include "ServerRequestParser.hpp"

#include "RequestMultipartBodyParser.hpp"
#include "RequestRawBodyParser.hpp"
#include "ServerRequest.hpp"
#include "ServerConfig.hpp"
#include "status.hpp"

ServerRequestParser::ServerRequestParser(t_request* request, const t_config* config, ServerLogger* logger)
	: 
	  _config(config),
	  _logger(logger),
	  _header_parser(config, logger),
	  _body_parser(NULL),
	  _is_cgi(false),
	  _header_parsed(false),
	  _request(request) {
}

ServerRequestParser::~ServerRequestParser() {
	if (_body_parser) {
		delete _body_parser;
		_body_parser = NULL;
	}
}

Status ServerRequestParser::parse_header(const std::string& content, std::string& body_out) {
	if (_header_parsed == true) {
		return Status::OK();
	}
	Status status;
	size_t cursor_pos;

	status = _header_parser.feed(content, cursor_pos);
	if (!status) {
		return status;
	}
	status = _header_parser.apply(*_request);
	if (!status || status.error() == DataIsNotReady) {
		return status;
	}

	if (cursor_pos != content.size()) {
		body_out = content.substr(cursor_pos);
	}

	_is_cgi = (_request->uri_path.find("cgi-bin/") != std::string::npos);
	if (_request->method == "POST" || _is_cgi == true) {
		create_body_parser();
	}

	_header_parsed = true;

	return Status::OK();
}

Status ServerRequestParser::parse_body(const std::string& content) {
	if (_header_parsed == false) {
		return Status::DataIsNotReady();
	} else if (_body_parser == NULL) {
		return Status::OK();
	}

	Status status;
	status = _body_parser->feed(content, 0);
	if (!status) {
		return status;
	}
	if (status != DataIsNotReady) {
		_body_parser->apply(*_request);
	}

	return Status::OK();
}

bool ServerRequestParser::is_cgi_request() const {
	return _is_cgi;
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

void ServerRequestParser::create_body_parser() {
	if (_is_cgi) {
		_body_parser = new RequestRawBodyParser(_request->content_length, InFile);
	} else if (_request->content_type.type == "multipart" &&
			   _request->content_type.subtype == "form-data") {
		const std::string* boundary = _request->content_type.find_parameter("boundary");
		_body_parser = new RequestMultipartParser(*boundary, _request->content_length, _logger);
	} else {
		_body_parser = new RequestRawBodyParser(_request->content_length, InBuffer);
	}
}
