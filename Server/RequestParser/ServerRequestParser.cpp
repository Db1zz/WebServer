#include "ServerRequestParser.hpp"

#include <stdlib.h>

#include <cassert>

#include "ServerRequest.hpp"
#include "ServerRequestParserHelpers.hpp"

/*
	POST /upload HTTP/1.1
	Host: example.com
	Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW
	Content-Length: 681

	------WebKitFormBoundary7MA4YWxkTrZu0gW
	Content-Disposition: form-data; name="username"

	vasya
	------WebKitFormBoundary7MA4YWxkTrZu0gW
	Content-Disposition: form-data; name="avatar"; filename="photo.png"
	Content-Type: image/png

	<данные бинарного файла photo.png>
	------WebKitFormBoundary7MA4YWxkTrZu0gW--
*/

ServerRequestParser::ServerRequestParser(t_request* request)
	: _request(request),
	  _cursor_pos(0),
	  _request_header_parsed(false),
	  _boundary_header_parsed(false) {
}

ServerRequestParser::ServerRequestParser(const ServerRequestParser& copy) {
	*this = copy;
}

ServerRequestParser& ServerRequestParser::operator=(const ServerRequestParser& copy) {
	if (this == &copy) {
		return *this;
	}

	_request = copy._request;
	_cursor_pos = copy._cursor_pos;
	_request_header_parsed = copy._request_header_parsed;
	_boundary_header_parsed = copy._boundary_header_parsed;
	_boundary_start = copy._boundary_start;
	_boundary_end = copy._boundary_end;
	_cache = copy._cache;

	return *this;
}

Status ServerRequestParser::parse_chunk(const std::string& chunk) {
	Status status;
	std::string header;
	_cache.append(chunk);

	// if (!_request_header_parsed) {
	// 	status = parse_request_header();
	// 	if (status) {
	// 		_request_header_parsed = true;
	// 	}
	// }
	// if (status && !_boundary_header_parsed) {
	// 	// status = internal_server_request_parser::parse_request_body_header(
	// 	// 	_cache, _cursor_pos, _request->transfered_length, _boundary_start, header);
	// 	if (status) {
	// 		get_filename_from_request_body(header, _request->filename);
	// 		get_mime_from_filename(_request->filename, _request->mime_type);
	// 		_boundary_header_parsed = true;
	// 	}
	// }
	// if (status && _boundary_header_parsed && !_request->is_request_ready()) {
	// 	// status = internal_server_request_parser::parse_request_body_chunk(
	// 	// 	_cache, _cursor_pos, _request->transfered_length, _boundary_end, _request->body_chunk);
	// }
	return status;
}

/*
	+-------------------+---------------------+
	|        Part       |       Data          |
	+-------------------+---------------------+
	|  Scheme           | https               |
	|  User             | bob                 |
	|  Password         | bobby               |
	|  Host             | www.lunatech.com    |
	|  Port             | 8080                |
	|  Path             | /file;p=1           |
	|  Path parameter   | p=1                 |
	|  Query            | q=2                 |
	|  Fragment         | third               |
	+-------------------+---------------------+

	https://bob:bobby@www.lunatech.com:8080/file;p=1?q=2#third
	\___/   \_/ \___/ \______________/ \__/\_______/ \_/ \___/
	|      |    |          |          |      |   \_/  |    |
	Scheme User Password    Host       Port  Path |   | Fragment
			\_____________________________/       | Query
						|               Path parameter
					Authority
*/
Status ServerRequestParser::parse_request_header() {
	Status status;
	std::string token_type;
	std::string token_value;
	size_t end;

	end = _cache.find("\r\n\r\n");
	if (end == std::string::npos) {
		return Status::Incomplete();
	}

	status = parse_request_line();
	if (!status) {
		return Status::BadRequest();
	}

	do {
		_cursor_pos = internal_server_request_parser::get_token_with_delims(_cache, _cursor_pos,
																			token_type, ": \n");
		_cursor_pos = internal_server_request_parser::get_token_with_delim(_cache, _cursor_pos,
																		   token_value, "\r\n");
		if (token_type == "Host") {
			_request->host = token_value;
		} else if (token_type == "User-Agent") {
			_request->user_agent = token_value;
		} else if (token_type == "Accept") {
			_request->accept = token_value;
		} else if (token_type == "Accept-Language") {
			_request->language = token_value;
		} else if (token_type == "Connection") {
			_request->connection = token_value;
		} else if (token_type == "Content-Length") {
			_request->content_length = atol(token_value.c_str());
			_request->transfered_length = 0;
		} else if (token_type == "Content-Type") {
			_request->content_type = token_value;
			parse_boundary();
			_boundary_start = "--" + _request->boundary + "\r\n";
			_boundary_end = "\r\n--" + _request->boundary + "--\r\n";
		}
	} while (_cursor_pos < end);
	return status;
}

bool ServerRequestParser::is_method_valid(const std::string& method) {
	return (method == "GET" || method == "POST" || method == "DELETE");
}

// TODO
// Status ServerRequestParser::normalize_uri(const t_commonConfig& common_server_config,
// 										  std::string& uri) {
// 	Status status;

// 	return Status();
// }

Status ServerRequestParser::parse_request_line() {
	size_t pos;

	pos = internal_server_request_parser::get_token_with_delims(_cache, _cursor_pos,
																_request->method, " ");
	if (!is_method_valid(_request->method)) {
		// TODO: we have to return BAD REQUEST if method has forbidden characters 
		return Status::NotImplemented();
	}
	pos = internal_server_request_parser::get_token_with_delims(_cache, pos, _request->uri_path,
																" ;?");
	if (_cache[pos] == ';') {
		pos = internal_server_request_parser::get_token_with_delims(_cache, pos, _request->uri_path,
																	" ?");
	}
	if (_cache[pos] == '?') {
		pos = internal_server_request_parser::get_token_with_delims(_cache, pos, _request->uri_path,
																	" ");
	}
	pos = internal_server_request_parser::get_token_with_delim(_cache, pos,
															   _request->protocol_version, "\r\n");
	// if (_request->method != "POST") {
	// 	internal_server_request_parser::parse_file_name_with_mime(
	// 		_request->uri_path, _request->filename, _request->mime_type);
	// }

	_cursor_pos = pos;
	return Status::OK();
}

Status ServerRequestParser::parse_boundary() {
	std::string boundary_key("boundary=");
	size_t start = _cache.find(boundary_key);

	if (start == std::string::npos) {
		return Status::RequestBoundaryIsNotProvided();
	}
	size_t end = _cache.find("\r", start);
	if (end == std::string::npos) {
		return Status::IncompleteRequestBodyHeader();
	}
	_request->boundary =
		std::string(_cache.begin() + start + boundary_key.size(), _cache.begin() + end);
	return Status::OK();
}

// 	https://datatracker.ietf.org/doc/html/rfc3986#section-3.3
//   pchar       = unreserved / pct-encoded / sub-delims / ":" / "@"
//   pct-encoded = "%" HEXDIG HEXDIG
//   unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
//   sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
// 			  / "*" / "+" / "," / ";" / "="

Status ServerRequestParser::get_filename_from_request_body(const std::string& request_string,
														   std::string& result) {
	std::string key("filename=\"");
	size_t start = request_string.find(key);
	if (start == std::string::npos) {
		return Status::NoFilename();
	}
	start += key.size();
	size_t end = request_string.find("\"", start);
	if (end == std::string::npos) {
		return Status::Incomplete();
	}
	result = std::string(request_string.begin() + start, request_string.begin() + end);
	return Status::OK();
}

Status ServerRequestParser::get_mime_from_filename(const std::string& filename,
												   std::string& result) {
	size_t mime_pos = filename.find(".");
	if (mime_pos == std::string::npos) {
		return Status::NoMime();
	}
	result = std::string(filename.begin() + mime_pos, filename.end());
	return Status::OK();
}