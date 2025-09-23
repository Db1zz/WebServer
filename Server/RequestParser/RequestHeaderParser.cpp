#include "RequestHeaderParser.hpp"

#include "RequestHeaderParserHelpers.hpp"

RequestHeaderParser::RequestHeaderParser() {
}

Status RequestHeaderParser::feed(const std::string& content, size_t start_pos) {
	_buffer.append(content.begin(), content.end());

	if (is_header_complete()) {
		return Status::OK();
	}
	return Status::Incomplete();
}

Status RequestHeaderParser::apply(t_request& request) {
	return parse_complete_header(request);
}

bool RequestHeaderParser::is_header_complete() {
	return _buffer.find("\r\n\r\n") != std::string::npos;
}

Status RequestHeaderParser::parse_complete_header(t_request& request) {
	Status status;
	size_t header_end = _buffer.find("\r\n\r\n");
	size_t pos;

	if (header_end == std::string::npos) {
		return Status::Incomplete();
	}

	status = parse_request_line();
	if (!status) {
		return Status::BadRequest();
	}

	do {
		std::string token_type;
		std::string token_value;

		pos =
			internal_server_request_parser::get_token_with_delims(_buffer, pos, token_type, ": \n");
		pos =
			internal_server_request_parser::get_token_with_delim(_buffer, pos, token_value, "\r\n");
		if (token_type == "Host") {
			request.host = token_value;
		} else if (token_type == "User-Agent") {
			request.user_agent = token_value;
		} else if (token_type == "Accept") {
			request.accept = token_value;
		} else if (token_type == "Accept-Language") {
			request.language = token_value;
		} else if (token_type == "Connection") {
			request.connection = token_value;
		} else if (token_type == "Content-Length") {
			request.content_length = atol(token_value.c_str());
			request.transfered_length = 0;
		} else if (token_type == "Content-Type") {
			request.content_type = token_value;
			parse_boundary(request);
		}
	} while (pos < header_end);
	return status;
}

Status RequestHeaderParser::parse_boundary(t_request& request) {
	std::string boundary_key("boundary=");
	size_t start = _buffer.find(boundary_key);

	if (start == std::string::npos) {
		return Status::RequestBoundaryIsNotProvided();
	}
	size_t end = _buffer.find("\r", start);
	if (end == std::string::npos) {
		return Status::IncompleteRequestBodyHeader();
	}
	request.boundary =
		std::string(_buffer.begin() + start + boundary_key.size(), _buffer.begin() + end);
	return Status::OK();
}

Status RequestHeaderParser::parse_request_line() {
	size_t pos;

	pos = internal_server_request_parser::get_token_with_delims(_buffer, _cursor_pos,
																_request->method, " ");
	if (!is_method_valid(_request->method)) {
		// TODO: we have to return BAD REQUEST if method has forbidden characters
		return Status::NotImplemented();
	}
	pos = internal_server_request_parser::get_token_with_delims(_buffer, pos, _request->uri_path,
																" ;?");
	if (_buffer[pos] == ';') {
		pos = internal_server_request_parser::get_token_with_delims(_buffer, pos,
																	_request->uri_path, " ?");
	}
	if (_buffer[pos] == '?') {
		pos = internal_server_request_parser::get_token_with_delims(_buffer, pos,
																	_request->uri_path, " ");
	}
	pos = internal_server_request_parser::get_token_with_delim(_buffer, pos,
															   _request->protocol_version, "\r\n");
	// if (_request->method != "POST") {
	// 	internal_server_request_parser::parse_file_name_with_mime(
	// 		_request->uri_path, _request->filename, _request->mime_type);
	// }

	_cursor_pos = pos;
	return Status::OK();
}

bool RequestHeaderParser::is_method_valid(const std::string& method) {
	return (method == "GET" || method == "POST" || method == "DELETE");
}

Status RequestHeaderParser::get_filename_from_request_body(const std::string& request_string,
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

Status RequestHeaderParser::get_mime_from_filename(const std::string& filename,
												   std::string& result) {
	size_t mime_pos = filename.find(".");
	if (mime_pos == std::string::npos) {
		return Status::NoMime();
	}
	result = std::string(filename.begin() + mime_pos, filename.end());
	return Status::OK();
}