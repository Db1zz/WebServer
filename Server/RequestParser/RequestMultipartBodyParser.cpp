
#include "RequestMultipartBodyParser.hpp"

#include "ServerRequestParserHelpers.hpp"

/*
	------WebKitFormBoundary7MA4YWxkTrZu0gW
	Content-Disposition: form-data; name="username"

	vasya
	------WebKitFormBoundary7MA4YWxkTrZu0gW
	Content-Disposition: form-data; name="avatar"; filename="photo.png"
	Content-Type: image/png

	<cool binary data :sunglasses:>
	------WebKitFormBoundary7MA4YWxkTrZu0gW
	Content-Disposition: form-data; name="avatar"; filename="photo.txt"
	Content-Type: text/txt

	dfsgsdfglksdjfg;lkdsjfg;lknsd;lfgjds;lkfgj
	------WebKitFormBoundary7MA4YWxkTrZu0gW--
*/

RequestMultipartParser::RequestMultipartParser(const std::string& boundary, int content_length)
	: _content_length(content_length), _data_size(0), _last_content(NULL), _is_end_boundary_found(false) {
	_start_boundary = "--" + boundary + "\r\n";
	_end_boundary = "\r\n--" + boundary + "--\r\n";
}

Status RequestMultipartParser::feed(const std::string& content, size_t start_pos) {
	Status status;
	_data_size += content.size() - start_pos;
	_buffer.append(content.substr(start_pos));
	do {

		size_t boundary_pos = search_boundary();
		if (boundary_pos != std::string::npos) {
			if (_is_end_boundary_found) {
				_last_content->data.append(_buffer.begin(), _buffer.begin() + boundary_pos);
				_last_content->is_finished = true;
				_buffer.clear();
				break;
			}
			if (_last_content != NULL && !_last_content->is_finished) {
				status = parse_and_finish_content(boundary_pos);
				if (!status) {
					break;
				}
				continue;
			}
			status = parse_body_with_header(boundary_pos);
		} else {
			status = parse_body_without_any_boundary();
		}
	} while (_buffer.size() > _end_boundary.size() && status && status.error() != DataIsNotReady);

	return status;
}

void RequestMultipartParser::apply(t_request& request) {
	while (!_content_data.empty()) {
		t_request_content& last_request_content = request.content_data.back();
		if (!request.content_data.empty() && _content_data.front().is_finished && !last_request_content.is_finished) {
			last_request_content.is_finished = true;
			last_request_content.data.append(_content_data.front().data);
			break;
		}
		
		if (!_content_data.front().is_finished) {
			if (!request.content_data.empty() && !last_request_content.is_finished) {
				last_request_content.data.append(_content_data.front().data);
			} else {
				request.content_data.push_back(_content_data.front());
			}
			_content_data.front().data.clear();
			break;
		}

		request.content_data.push_back(_content_data.front());
		_content_data.pop_front();
	}
	request.transfered_length = _data_size;
	std::cout << "ABOBA: " << request.transfered_length << std::endl;
}

size_t RequestMultipartParser::search_boundary() {
	size_t boundary_pos = _buffer.find(_start_boundary);
	if (boundary_pos == std::string::npos) {
		boundary_pos = _buffer.find(_end_boundary);
		if (boundary_pos != std::string::npos) {
			_is_end_boundary_found = true;
		}
	}
	return boundary_pos;
}

Status RequestMultipartParser::parse_body_without_any_boundary() {
	if (_end_boundary.size() > _buffer.size()) {
		return Status::DataIsNotReady();
	}

	size_t content_size = _buffer.size() - _end_boundary.size();
	_last_content->data.append(_buffer.begin(), _buffer.begin() + content_size);

	_buffer.erase(0, content_size);
	return Status::DataIsNotReady();
}

Status RequestMultipartParser::parse_body_with_header(size_t boundary_pos) {
	static const std::string header_end_key("\r\n\r\n");
	Status status;
	size_t header_end = _buffer.find(header_end_key);

	if (header_end == std::string::npos) {
		return Status::DataIsNotReady();
	}
	// if (_buffer.size() >= MAX_REQUEST_BODY_HEADER_LENGTH) {
	// 	return Status::RequestEntityTooLarge();
	// }
	update_last_content();
	status = parse_body_header(boundary_pos);
	_buffer = _buffer.substr(header_end + header_end_key.size());
	return status;
}

Status RequestMultipartParser::parse_and_finish_content(size_t boundary_pos) {
	if (!(_buffer[boundary_pos - 2] == '\r' && _buffer[boundary_pos - 1] == '\n')) {
		return Status::BadRequest();
	}
	_last_content->data.append(_buffer.substr(0, boundary_pos - 2));
	_buffer = _buffer.substr(boundary_pos);
	_last_content->is_finished = true;
	return Status::OK();
}

Status RequestMultipartParser::parse_body_header(size_t boundary_pos) {
	Status status;
	size_t pos = boundary_pos + _start_boundary.size();
	std::string content_disposition;
	std::string content_type;
	pos = internal_server_request_parser::get_token_with_delim(_buffer, pos, content_disposition,
															   "\r\n", true);
	status = parse_filename(content_disposition, _last_content->filename);
	if (status == BadRequest) {
		return status;
	}

	status = parse_name(content_disposition, _last_content->name);
	if (status == BadRequest) {
		return status;
	}

	pos = internal_server_request_parser::get_token_with_delim(_buffer, pos, content_type, "\r\n", true);
	status = parse_content_type(content_type, _last_content->content_type);
	if (status == NoMime) {
		return Status::OK();
	}

	return status;
}

Status RequestMultipartParser::parse_name(const std::string& content_disposition,
										  std::string& name) {
	const std::string name_key("name=\"");
	size_t start = content_disposition.find(name_key);
	if (start == std::string::npos) {
		return Status::BadRequest();
	}

	start += name_key.size();
	size_t end = content_disposition.find("\"", start);
	if (end == std::string::npos) {
		return Status::BadRequest();
	}
	name = content_disposition.substr(start, end - start);
	return Status::OK();
}

Status RequestMultipartParser::parse_content_type(const std::string& content_type, std::string& mime) {
	const std::string content_type_key("Content-Type: ");

	size_t start = content_type.find(content_type_key);
	if (start == std::string::npos) {
		return Status::NoMime();
	}

	size_t slash_pos = content_type.find("/");
	if (slash_pos == std::string::npos) {
		return Status::NoMime();
	}

	if (slash_pos - start > MIME_TYPE_MAX_SIZE || content_type.size() - slash_pos > MIME_SUBTYPE_MAX_SIZE) {
		return Status::RequestHeaderFieldsTooLarge();
	}

	if (!internal_server_request_parser::is_string_valid_token(content_type.c_str() + start, slash_pos - start)) {
		return Status::BadRequest();
	}

	if (!internal_server_request_parser::is_string_valid_token(content_type.c_str() + slash_pos + 1)) {
		return Status::BadRequest();
	}

	mime = content_type.substr(start, content_type.size() - start);

	return Status::OK();
}

static void skip_ows_and_folds(const std::string& line, size_t& pos) {
	const size_t len = line.size();

	while (pos < len) {
		if (pos + 2 < len && internal_server_request_parser::is_crlf(line.c_str() + pos) && internal_server_request_parser::is_ws(line[pos + 2])) {
			pos += 3;
			while (pos < len && internal_server_request_parser::is_ws(line[pos])) {
				++pos;
			}
			continue;
		}

		if (!internal_server_request_parser::is_ws(line[pos])) {
			break;
		}

		++pos;
	}
}

Status RequestMultipartParser::parse_filename(const std::string& line, std::string& filename) {
	const std::string filename_key("filename");
	Status status;
	size_t pos;

	pos = line.find(filename_key);
	if (pos == std::string::npos) {
		return Status::NoFilename();
	}

	pos += filename_key.size();
	pos = pos;

	skip_ows_and_folds(line, pos);
	if (line[pos] != '=') {
		return Status::BadRequest();
	}
	++pos; // aboba skip '='
	skip_ows_and_folds(line, pos);

	if (line[pos] == '\"') {
		status = internal_server_request_parser::parse_quoted_string(line, pos, pos, filename);
	} else {
		status = parse_unquoted_filename(line, pos, filename);
	}

	return status;
}

Status RequestMultipartParser::parse_unquoted_filename(const std::string& line, size_t pos, std::string& filename) {
	size_t end;

	end = line.find("\r\n", pos);
	if (end == std::string::npos) {
		end = line.find("; ", pos);
	}

	if (end == std::string::npos ||
		!internal_server_request_parser::is_string_valid_token(line.c_str() + pos, end)) {
		return Status::BadRequest();
	}

	filename = line.substr(pos, end);
	if (filename.empty()) {
		return Status::BadRequest();
	}
	return Status::OK();
}

void RequestMultipartParser::update_last_content() {
	if (_content_data.empty() || _content_data.back().is_finished) {
		_content_data.push_back(t_request_content());
		_last_content = &_content_data.back();
	}
}


