#include "ServerRequestParser.hpp"

#include <stdlib.h>

#include <cassert>

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
	|      |    |          |          |      | \_/  |    |
	Scheme User Password    Host       Port  Path |   | Fragment
			\_____________________________/       | Query
						|               Path parameter
					Authority
*/
Status ServerRequestParser::parse_request_header(std::string& request_string, t_request& request, ServerLogger& server_logger) {
	Status status;

	status = get_request_line(request_string, request);
	if (!status) {
		server_logger.log_error("ServerRequestParser::parse_request_header()", status.msg());
		return Status("Bad Request", 400, false);
	}

	status = get_request_headers(request_string, request);
	if (!status) {
		server_logger.log_error("ServerRequestParser::parse_request_header()", status.msg());
		return Status("Bad Request", 400, false);
	}

	return Status();
}

Status ServerRequestParser::parse_request_body_chunk(t_request& request) {
	Status status;
	std::string request_body_header;

	// REMOVEME
	if (request.method == "POST" && extract_request_body_header(request, request_body_header)) {
		get_filename_from_request_body(request_body_header, request.filename);
		get_mime_from_filename(request.filename, request.mime_type);
	}
	erase_request_body_end_boundary(request);

	return Status();
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

Status ServerRequestParser::get_request_line(std::string& request_string, t_request& request) {
	std::string method;
	std::string uri_path;
	std::string uri_path_params;
	std::string protocol_version;
	std::string cgi_query_string;

	get_token(request_string, method, " ");
	if (!is_method_valid(method)) {
		return Status("client sent invalid method while reading client request line");
	}
	get_token(request_string, uri_path, " ;?");
	if (request_string[0] == ';') {
		get_token(request_string, uri_path, " ?");
	}
	if (request_string[0] == '?') {
		get_token(request_string, uri_path_params, " ");
	}
	get_token(request_string, protocol_version, "\n\0");

	request.method = method;
	request.uri_path = uri_path;
	request.uri_path_params = uri_path_params;
	request.protocol_version = protocol_version;
	request.cgi_query_string = cgi_query_string;

	// REMOVEME
	if (request.method != "POST") {
		extract_file_name_with_mime(request.uri_path, request.filename, request.mime_type);
	}

	return Status();
}

Status ServerRequestParser::get_request_headers(std::string& request_string, t_request& request) {
	Status status;
	std::string token_type;
	std::string token_value;

	do {
		get_token(request_string, token_type, ": \n");
		get_token(request_string, token_value, "\n\0");
		if (token_type == "Host") {
			request.host = token_value;
		} else if (token_type == "User-Agent") {
			request.user_agent = token_value;
		} else if (token_type == "Accept") {
			request.accept = token_value;
			if (*(request.accept.end() - 1) == '\r') {
				request.accept.erase(request.accept.size() - 1);
			}
		} else if (token_type == "Accept-Language") {
			request.language = token_value;
		} else if (token_type == "Connection") {
			request.connection = token_value;
		} else if (token_type == "Content-Length") {
			request.content_length = atol(token_value.c_str());
		} else if (token_type == "Content-Type") {
			request.content_type = token_value;
			get_boundary(request.content_type, request);
		}
	} while (!request_string.empty() && request_string[0] != '\r');
	get_token(request_string, token_type, "\n");

	return status;
}

// 	https://datatracker.ietf.org/doc/html/rfc3986#section-3.3
//   pchar       = unreserved / pct-encoded / sub-delims / ":" / "@"
//   pct-encoded = "%" HEXDIG HEXDIG
//   unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
//   sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
// 			  / "*" / "+" / "," / ";" / "="

bool ServerRequestParser::is_unreserved(char c) {
	return (isdigit(c) || isalpha(c) || c == '-' || c == '.' || c == '_' || c == '~');
}

bool ServerRequestParser::is_sub_delims(char c) {
	return (c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' || c == '*' ||
			c == '+' || c == ',' || c == ';' || c == '=');
}

bool ServerRequestParser::is_path_valid(const std::string& path) {
	const size_t path_size = path.size();

	for (size_t i = 0; i < path_size; ++i) {
		if (is_sub_delims(path[i]) || is_unreserved(path[i]) || path[i] == ':' || path[i] == '@') {
			continue;
		} else if (is_pos_hex(path, i)) {
			i += 3;
			continue;
		}
		return false;
	}
	return true;
}

bool ServerRequestParser::is_pos_hex(const std::string& str, size_t pos) {
	if (str[pos] == '%' && str.size() - pos > 2) {
		std::string hex(str.begin() + pos, str.begin() + pos + 2);
		if (hex.find_first_not_of("0123456789abcdefABCDEF", 1) == std::string::npos) {
			return true;
		}
	}
	return false;
}

std::string& ServerRequestParser::get_token(std::string& request_string, std::string& result,
											const char* delims) {
	size_t pos = request_string.find_first_of(delims);
	if (pos == std::string::npos) {
		result = request_string;
		request_string.erase();
		return request_string;
	}

	result = std::string(request_string.begin(), request_string.begin() + pos);
	pos = request_string.find_first_not_of(delims, pos);
	if (pos == std::string::npos) {
		request_string.erase();
	} else {
		request_string.erase(0, pos);
	}

	return request_string;
}

std::string& ServerRequestParser::consume_char(std::string& request_string, char c) {
	if (request_string[0] == c) {
		request_string.erase(0);
	}
	return request_string;
}

Status ServerRequestParser::extract_file_name_with_mime(const std::string& uri_path,
														std::string& filename,
														std::string& mime_type) {
	size_t slash_pos = uri_path.find_last_of("/");
	size_t mime_pos;
	if (slash_pos == std::string::npos) {
		return Status("invalid path");
	}

	filename = std::string(uri_path.begin() + slash_pos, uri_path.end());

	mime_pos = filename.find(".");
	if (mime_pos != std::string::npos) {
		mime_type = std::string(filename.begin() + mime_pos, filename.end());
	}
	return Status();
}

Status ServerRequestParser::get_filename_from_request_body(const std::string& request_string, std::string& result) {
	std::string key("filename=\"");
	size_t start = request_string.find(key);
	if (start == std::string::npos) {
		return Status("request body has no filename key");
	}
	start += key.size();
	size_t end = request_string.find("\"", start);
	if (end == std::string::npos) {
		return Status("filename key found but the name of the file is incomplete");
	}
	result = std::string(request_string.begin() + start, request_string.begin() + end);
	return Status();
}

Status ServerRequestParser::get_mime_from_filename(const std::string& filename,
												   std::string& result) {
	size_t mime_pos = filename.find(".");
	if (mime_pos == std::string::npos) {
		return Status("filename has not mime type");
	}
	result = std::string(filename.begin() + mime_pos, filename.end());
	return Status();
}

Status ServerRequestParser::extract_request_body_header(t_request& request, std::string& result) {
	std::string body_header_end("\r\n\r\n");
	size_t start = request.cache.find(request.boundary + "\r\n");
	size_t end = request.cache.find(body_header_end);
	
	if (start == std::string::npos || end == std::string::npos) {
		return Status("incomplete request body header");
	}

	if (end == std::string::npos) {
		return Status("incomplete request body header");
	}
	end += body_header_end.size();
	result = std::string(request.cache.begin() + start, request.cache.begin() + end);
	request.cache.erase(start, end);

	return Status();
}

Status ServerRequestParser::erase_request_body_end_boundary(t_request& request) {
	std::string request_boundary_end("\r\n" + request.boundary + "--\r\n");

	if (request.cache.size() < request_boundary_end.size()) {
		return Status("boundary not found");
	}
	size_t size_delta = request.cache.size() - request_boundary_end.size();
	size_t boundary_pos = request.cache.find(request_boundary_end);

	if (boundary_pos == std::string::npos) {
		request.body_chunk = std::string(request.cache.begin(), request.cache.begin() + size_delta);
		request.cache =
			std::string(request.cache.begin() + size_delta,
						request.cache.begin() + size_delta + request_boundary_end.size());
		return Status("boundary not found");
	}

	request.body_chunk = 
		std::string(request.cache.begin(), request.cache.begin() + boundary_pos) +
		std::string(request.cache.begin() + boundary_pos + request_boundary_end.size(), request.cache.end());
	if (request.cache.empty()) {
		request.cache.erase();
	}
	return Status();
}

Status ServerRequestParser::get_boundary(const std::string& request_string, t_request& request) {
	std::string boundary_key("boundary=");
	size_t start = request_string.find(boundary_key);

	if (start == std::string::npos) {
		return Status("boundary was not provided in content-type");
	}
	size_t end = request_string.find("\r", start);
	if (end == std::string::npos) {
		return Status("incomplete header or invalid header format");
	}
	request.boundary = "--" + std::string(request_string.begin() + start + boundary_key.size(), request_string.begin() + end);
	return Status();
}