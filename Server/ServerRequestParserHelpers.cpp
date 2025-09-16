#include "ServerRequestParserHelpers.hpp"

#include <iostream>

namespace internal_server_request_parser {
bool is_unreserved(char c) {
	return (isdigit(c) || isalpha(c) || c == '-' || c == '.' || c == '_' || c == '~');
}

bool is_sub_delims(char c) {
	return (c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' || c == '*' ||
			c == '+' || c == ',' || c == ';' || c == '=');
}

bool is_path_valid(const std::string& path) {
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

bool is_pos_hex(const std::string& str, size_t pos) {
	if (str[pos] == '%' && str.size() - pos > 2) {
		std::string hex(str.begin() + pos, str.begin() + pos + 2);
		if (hex.find_first_not_of("0123456789abcdefABCDEF", 1) == std::string::npos) {
			return true;
		}
	}
	return false;
}

size_t get_token_with_delims(const std::string& data, size_t start, std::string& result,
							 const char* delims) {
	size_t end = data.find_first_of(delims, start);
	if (end == std::string::npos) {
		result = data;
		return result.size();
	}
	result = std::string(data.begin() + start, data.begin() + end);
	end = data.find_first_not_of(delims, end);
	if (end != std::string::npos) {
		return end;
	}

	return start + result.size();
}

size_t get_token_with_delim(const std::string& data, size_t start, std::string& result,
							const char* delim) {
	size_t end = data.find(delim, start);
	if (end == std::string::npos) {
		result = data;
		return result.size();
	}
	result = std::string(data.begin() + start, data.begin() + end);
	end = data.find_first_not_of(delim, end);
	if (end != std::string::npos) {
		return end;
	}

	return start + result.size();
}

std::string& consume_char(std::string& request_string, char c) {
	if (request_string[0] == c) {
		request_string.erase(0);
	}
	return request_string;
}

Status parse_file_name_with_mime(const std::string& uri_path, std::string& filename,
								 std::string& mime_type) {
	size_t slash_pos = uri_path.find_last_of("/");
	size_t mime_pos;
	if (slash_pos == std::string::npos) {
		return Status::InvalidPath();
	}

	filename = std::string(uri_path.begin() + slash_pos, uri_path.end());

	mime_pos = filename.find(".");
	if (mime_pos != std::string::npos) {
		mime_type = std::string(filename.begin() + mime_pos, filename.end());
	}
	return Status::OK();
}

Status parse_request_body_header(const std::string& cache, size_t& pos, size_t& transfered_length,
								 const std::string& boundary_start, std::string& result) {
	std::string body_header_end("\r\n\r\n");
	size_t start = cache.find(boundary_start, pos);
	size_t end = cache.find(body_header_end, start);

	if (start == std::string::npos) {
		return Status::IncompleteRequestBodyHeader();
	} else if (end == std::string::npos) {
		return Status::IncompleteRequestBodyHeader();
	}

	end += body_header_end.size();
	result = std::string(cache.begin() + start, cache.begin() + end);
	pos = end;
	transfered_length += result.size();
	return Status::OK();
}

Status parse_request_body_chunk(std::string& cache, size_t& pos, size_t& transfered_length,
								const std::string& boundary_end, std::string& result) {
	if (cache.size() - pos < boundary_end.size()) {
		return Status::EndBoundaryNotFound();
	}
	size_t size_delta = cache.size() - pos - boundary_end.size();
	size_t boundary_pos = cache.find(boundary_end, pos);
	std::string::iterator start = cache.begin() + pos;

	if (boundary_pos == std::string::npos) {
		result = std::string(start, start + size_delta);
		cache = std::string(start + size_delta, start + size_delta + boundary_end.size());
		pos = 0;
		transfered_length += result.size();
		return Status::EndBoundaryNotFound();
	}

	result = std::string(start, cache.begin() + boundary_pos);
	cache = std::string(cache.begin() + boundary_pos + boundary_end.size(), cache.end());
	transfered_length += result.size() + boundary_end.size();
	pos = 0;
	return Status::OK();
}
} // namespace internal_server_request_parser