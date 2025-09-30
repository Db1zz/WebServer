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

/*
	Implementing CGI
	The CGI Routine:
		Questions: 
		1. Do we need to read the whole object body before passing it in a cgi process???

		2. where do we store request data? 
		2.1 UPD 1 Storing file in a temporary file is not the best thing, the response class will copy the whole thing in order to create 
		2.2 UPD 2 moving files within same partition in linux is instant. We can use this advantage.
		2.3 UPD 3 we need to check if we can change name of the file and move it around.
		2.4 UPD 4 we cannot rename file, the subject doesn't allow us to change it.
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