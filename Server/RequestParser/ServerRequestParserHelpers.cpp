#include "ServerRequestParserHelpers.hpp"

#include <stdlib.h>

#include <climits>
#include <iostream>

namespace internal_server_request_parser {
bool is_unreserved(char c) {
	return (isdigit(c) || isalpha(c) || c == '-' || c == '.' || c == '_' || c == '~');
}

bool is_sub_delim(char c) {
	return (c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' || c == '*' ||
			c == '+' || c == ',' || c == ';' || c == '=');
}

bool is_path_valid(const std::string& path) {
	const size_t path_size = path.size();

	for (size_t i = 0; i < path_size; ++i) {
		if (is_sub_delim(path[i]) || is_unreserved(path[i]) || path[i] == ':' || path[i] == '@') {
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
	if (pos + 3 <= str.size() && str[pos] == '%') {
		std::string hex(str.begin() + pos + 1, str.begin() + pos + 3);
		if (hex.find_first_not_of("0123456789abcdefABCDEF", 1) == std::string::npos) {
			return true;
		}
	}
	return false;
}

bool is_string_valid_token(const char* c, size_t len) {
	if (len == 0) {
		len = UINT_MAX;
	}

	for (size_t i = 0; c[i] != '\0' && len < i; ++i) {
		if (is_char_tspecial(c[i]) || is_char_ctl(c[i])) {
			return false;
		}
	}
	return true;
}

bool is_string_valid_token(const char* c) {
	return is_string_valid_token(c, 0);
}

bool is_char_tspecial(unsigned char c) {
	static const std::string tspecials("()<>@,;:\\\"/[]?=");
	return tspecials.find(c) != std::string::npos;
}

bool is_char_ctl(unsigned char c) {
	return (c <= 31) || (c == 127);
}

bool is_vchar(unsigned char c) {
	return c >= 33 && c <= 126;
}

bool is_obs_text(unsigned char c) {
	return c >= 128 && c <= 255;
}

bool is_ws(unsigned char c) {
	return c == ' ' || c == '	';
}

bool is_obs_fold(const char* c) {
	return c[0] == '\r' && c[1] == '\n' && is_ws(c[2]);
}

bool is_crlf(const char* c) {
	return (*c == '\r') && (*(c + 1) == '\n');
}

bool is_ows(const char* c) {
	return ((*c == '\r') && (*(c + 1) == '\n') && is_ws(*(c + 2))) || is_ws(*(c));
}

bool is_qd_text(unsigned char c) {
	return c == '!' || (c >= 35 && c <= 91) || (c >= 93 && c <= 126) || is_obs_text(c) || is_ws(c);
}

bool is_pchar(char c) {
	return is_unreserved(c) || is_sub_delim(c) || c == ':' || c == '@';
}

size_t get_token_with_delims(const std::string& data, size_t start, std::string& result,
							 const char* delims, bool skip_delims) {
	size_t end = data.find_first_of(delims, start);
	if (end == std::string::npos) {
		result = data.substr(start);
		return result.size() + start;
	}
	result = std::string(data.begin() + start, data.begin() + end);
	if (skip_delims) {
		end = data.find_first_not_of(delims, end);
		if (end != std::string::npos) {
			return end;
		}
	}
	return result.size() + start;
}

void unfold_string(std::string& string) {
	const size_t len = string.size();
	std::string buffer;
	size_t pos = 0;

	while (pos < len) {
		if (pos + 2 < len && is_obs_fold(string.c_str() + pos)) {
			pos += 3;
			buffer.push_back(' ');
			continue;
		}
		buffer.push_back(string[pos]);
		++pos;
	}
	string = buffer;
}

size_t get_token_with_delim(const std::string& data, size_t start, std::string& result,
							const char* delim, bool skip_delims) {
	size_t end = data.find(delim, start);
	if (end == std::string::npos) {
		std::cout << "size: " << data.size() << " start: " << start << std::endl;
		result = data.substr(start);
		return result.size() + start;
	}
	result = std::string(data.begin() + start, data.begin() + end);
	if (skip_delims) {
		end = data.find_first_not_of(delim, end);
		if (end != std::string::npos) {
			return end;
		}
	}

	return start + result.size();
}

Status parse_nonencoded_filename(const std::string& filename) {
	Status status;

	return status;
}

// syntax: <charset>'<language>'<percent-encoded-octets>
// the file name must already be separated from the path part, without the leading / character
// before the name
Status parse_encoded_filename(const std::string& encoded_filename, std::string& decoded_filename) {
	Status status;

	std::string charset;
	std::string language;
	std::string filename;

	size_t open_quote_pos = encoded_filename.find("\'");
	if (open_quote_pos == std::string::npos) {
		return Status::BadRequest();
	}

	size_t close_quote_pos = encoded_filename.find("\'", open_quote_pos + 1);
	if (close_quote_pos == std::string::npos) {
		return Status::BadRequest();
	}

	charset = encoded_filename.substr(0, open_quote_pos);
	if (close_quote_pos - open_quote_pos > 1) {
		language = encoded_filename.substr(open_quote_pos + 1, close_quote_pos);
	}

	filename = encoded_filename.substr(close_quote_pos + 1);

	status = decode_filename(charset, language, filename, decoded_filename);

	return status;
}

Status decode_filename(const std::string& charset, const std::string& language,
					   const std::string& encoded_filename, std::string& decoded_filename) {
	Status status;
	std::string decoded_buffer;

	size_t i = 0;
	while (i < encoded_filename.size()) {
		if (!is_pos_hex(encoded_filename, i)) {
			decoded_buffer += encoded_filename[i];
			++i;
			continue;
		}

		size_t consumed_chars = 0;
		std::string decoded_sequence;

		status = utf8_char_decoder(encoded_filename, i, decoded_sequence, consumed_chars);
		if (!status) {
			return status;
		}

		decoded_buffer.append(decoded_sequence);
		i += consumed_chars;
	}

	decoded_filename = decoded_buffer;
	return status;
}

size_t char_to_hex(char c) {
	static const std::string hex_vals = "0123456789abcdef";
	return hex_vals.find(std::tolower(c));
}

// 	диапазоны байтов включают в себя начало и конец ABOBA
// 	0xxxxxxx (00–7F)	ASCII	1 байт
// 	110xxxxx (C2–DF)	начало 2ex-байтовой буквы	2 байта
// 	1110xxxx (E0–EF)	начало 3ex-байтовой буквы	3 байта
// 	11110xxx (F0–F4)	начало 4ex-байтовой буквы	4 байта
Status utf8_char_decoder(const std::string& string, size_t char_pos, std::string& decoded,
						 size_t& consumed_chars) {
	u_int8_t leading_byte = static_cast<u_int8_t>(char_to_hex(string[char_pos + 1]) * 16 +
												  char_to_hex(string[char_pos + 2]));
	size_t continuation_byte_size = 0;
	size_t curr_char_pos = char_pos;
	std::string buffer;

	if (leading_byte >= static_cast<u_int8_t>('\xC2') &&
		leading_byte <= static_cast<u_int8_t>('\xDF')) {
		continuation_byte_size = 1;
	} else if (leading_byte >= static_cast<u_int8_t>('\xE0') &&
			   leading_byte <= static_cast<u_int8_t>('\xEF')) {
		continuation_byte_size = 2;
	} else if (leading_byte >= static_cast<u_int8_t>('\xF0') &&
			   leading_byte <= static_cast<u_int8_t>('\xF4')) {
		continuation_byte_size = 3;
	} else {
		// TODO utf8_decoder error?
		return Status::BadRequest();
	}

	buffer += static_cast<char>(leading_byte);

	while (continuation_byte_size != 0) {
		curr_char_pos += 3;
		if (!is_pos_hex(string, curr_char_pos)) {
			// TODO utf8_decoder error?
			return Status::BadRequest();
		}
		buffer += static_cast<char>(char_to_hex(string[curr_char_pos + 1]) * 16 +
									char_to_hex(string[curr_char_pos + 2]));
		--continuation_byte_size;
	}
	consumed_chars = (curr_char_pos + 3) - char_pos;
	decoded = buffer;
	return Status::OK();
}

bool is_quoted_pair_char(unsigned char c) {
	return (c == ' ' || c == '	') || is_vchar(c) || is_obs_text(c);
}

Status parse_quoted_string(const std::string& s, size_t pos, size_t& next_pos, std::string& out) {
	const size_t ows_last_char_pos = 2;
	const size_t escape_last_char_pos = 1;
	const size_t len = s.size();
	std::string buffer;

	if (s[pos] != '\"') {
		return Status::InvalidFilenameFormat();
	}
	++pos;

	while (pos < len && s[pos] != '\"') {
		if (pos + escape_last_char_pos < len && s[pos] == '\\') {
			if (!is_quoted_pair_char(s[pos + escape_last_char_pos])) {
				return Status::InvalidFilenameFormat();
			}
			buffer.push_back(s[pos + escape_last_char_pos]);
			pos += escape_last_char_pos + 1;
			continue;
		}
		if (pos + ows_last_char_pos < len && is_ows(s.c_str() + pos)) {
			pos = s.find_first_not_of("	 ", pos + ows_last_char_pos + 1);
			if (pos == std::string::npos) {
				return Status::InvalidFilenameFormat();
			}
			buffer.push_back(' ');
			continue;
		} else if (!is_qd_text(s[pos])) {
			return Status::InvalidFilenameFormat();
		}
		buffer.push_back(s[pos]);
		++pos;
	}
	if (s[pos] != '\"') {
		return Status::InvalidFilenameFormat();
	}
	next_pos = pos;
	out = buffer;
	return Status::OK();
}

void extract_filename(const std::string& full_path, std::string& out) {
	size_t last_slash_pos = 0;

	last_slash_pos = full_path.find_last_of("/");
	if (last_slash_pos == std::string::npos) {
		return;
	}
	out = full_path.substr(last_slash_pos + 1);
}

void extract_mime(const std::string& filename, std::string& out) {
	size_t last_slash_pos = 0;

	last_slash_pos = filename.find_last_of(".");
	if (last_slash_pos == std::string::npos) {
		return;
	}
	out = filename.substr(last_slash_pos);
}

} // namespace internal_server_request_parser