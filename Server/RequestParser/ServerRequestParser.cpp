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
	  _header_parser(logger),
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

/*
	Implementing CGI
	The CGI Routine:
		Questions:
		1. Do we need to read the whole object body before passing it in a cgi process???

		2. where do we store request data?
		2.1 UPD 1 Storing file in a temporary file is not the best thing, the response class will
   copy the whole thing in order to create 2.2 UPD 2 moving files within same partition in linux is
   instant. We can use this advantage. 2.3 UPD 3 we need to check if we can change name of the file
   and move it around. 2.4 UPD 4 we cannot rename file, the subject doesn't allow us to change it.
		2.5 Answer - store data in tiny chunks = bullshit, because then we have to implement
			stream consumer in CGI script that will handle decoding and storing somewhere the data.

		1. parse HTTP header
		2. find cgi-bin folder in uri_path
		3. setup pipes(?)
		4. fork new process
		5. in new process set env variables and execute cgi file
		5.1 in main process register in epoll a new process with the timeout timer
		6. read the data in a temp file, if the data is chunked the server should unchunk it first
		   before actually storing it in a temporary file.
		7. when process finished, send an response with the result(?)

		okay, my setps to work:

		1. Write a special parser for CGI that will handle data
		2. set env variables for a process(should they be set by child or parent process?)
		3. Setup routine that will fork and register new process
		4. send response when cgi finished or failed.
*/
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
		_body_parser = new RequestMultipartParser(*boundary, _request->content_length);
	} else {
		_body_parser = new RequestRawBodyParser(_request->content_length, InBuffer);
	}
}
