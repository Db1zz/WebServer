#include "CGIResponseParser.hpp"

#include <algorithm>
#include <map>

#include "RequestParser/ServerRequestParserHelpers.hpp"
#include "ServerLogger.hpp"

CGIResponseParser::CGIResponseParser(t_request* request, ServerLogger* logger)
	: _header_parsed(false), _logger(logger), _request(request) {
}

Status CGIResponseParser::parse_media_type(const std::string& field_value, t_media_type& media_type,
										   size_t pos, size_t& separator_pos) {
	std::string type_with_subtype;
	std::string type;
	std::string subtype;
	size_t slash_pos = 0;

	pos = internal_server_request_parser::get_token_with_delims(field_value, pos, type_with_subtype,
																";,", true);
	if (type_with_subtype.empty()) {
		log_error("RequestHeaderParser::parse_media_type",
				  std::string("media type is missing: '") + field_value.substr(pos) + "'");
		return Status::BadRequest();
	} else if (pos != field_value.size()) {
		--pos; // do not consume separator.
	}

	slash_pos = internal_server_request_parser::get_token_with_delim(type_with_subtype, slash_pos,
																	 type, "/", false);
	if (slash_pos == type_with_subtype.size()) {
		log_error("RequestHeaderParser::parse_media_type",
				  std::string("char '/' is missing: '") + type_with_subtype + "'");
		return Status::BadRequest();
	}

	subtype = type_with_subtype.substr(slash_pos + 1);
	if (type_with_subtype.empty()) {
		log_error("RequestHeaderParser::parse_media_type",
				  std::string("subtype is missing: '") + type_with_subtype + "'");
		return Status::BadRequest();
	}

	if (!(type == "text" || type == "image" || type == "audio" || type == "video" ||
		  type == "application" || type == "multipart" || type == "message" || type == "font")) {
		// */* == any
		type = "*";
		subtype = "*";
	}

	media_type.type = type;
	media_type.subtype = subtype;
	separator_pos = pos;
	return Status::OK();
}

bool CGIResponseParser::is_valid_status_code(const std::string& status_str) {
	const size_t len = status_str.size();

	if (status_str.empty()) {
		log_error("CGIResponseParser::is_valid_status_code", "status_str is empty");
		return false;
	}

	for (size_t i = 0; i < len; ++i) {
		if (!std::isdigit(status_str[i])) {
			log_error("CGIResponseParser::is_valid_status_code",
					  std::string("the char is not digit: '") + status_str[i] + "'");
			return false;
		}
	}

	int v = std::atoi(status_str.c_str());

	return v >= 100 && v <= 599;
}

Status CGIResponseParser::parse_status(const std::string& field_value, t_request& request) {
	Status status;
	size_t pos = 0;

	internal_server_request_parser::skip_ws(field_value, pos);
	std::string status_str;
	pos = internal_server_request_parser::get_token_with_delim(field_value, pos, status_str, " ",
															   true);
	if (is_valid_status_code(status_str) == false) {
		log_error("CGIResponseParser::parse_status",
				  std::string("the string is not valid number: '") + status_str + "'");

		return Status::BadRequest(); // TODO CHANGE ME?
	}

	request.status_string = field_value;

	return Status::OK();
}

Status CGIResponseParser::parse_parameter(const std::string& field_value, std::string& out,
										  size_t pos, size_t& separator_pos) {
	Status status;
	std::string buffer;
	size_t end = 0;

	if (field_value[pos] == '\"') {
		status = internal_server_request_parser::parse_quoted_string(field_value, pos, end, buffer);
		if (!status) {
			return status;
		}
	} else {
		pos = internal_server_request_parser::get_token_with_delims(field_value, pos, buffer, ";,",
																	false);
		if (buffer.empty()) {
			log_error("RequestHeaderParser::parse_parameter", std::string("value is empty'"));
			return Status::BadRequest();
		}
		if (!internal_server_request_parser::is_string_valid_token(buffer.c_str(), buffer.size())) {
			log_error("RequestHeaderParser::parse_parameter",
					  std::string("value is not a valid token: '") + buffer + "'");
			return Status::BadRequest();
		}
	}

	separator_pos = pos;
	out = buffer;

	return Status::OK();
}

Status CGIResponseParser::parse_content_type(const std::string& field_value, t_request& request) {
	if (field_value.empty()) {
		log_error("RequestHeaderParser::parse_content_type", std::string("value is empty"));
		return Status::BadRequest();
	}

	const size_t len = field_value.size();
	Status status;
	size_t pos = 0;
	t_media_type media_type;

	status = parse_media_type(field_value, media_type, pos, pos);
	if (!status) {
		log_error("RequestHeaderParser::parse_content_type",
				  std::string("invalid media type: '") + field_value + "'");
		return Status::BadRequest();
	}

	internal_server_request_parser::skip_ws(field_value, pos);
	while (pos < len && field_value[pos] == ';') {
		size_t equal_sign_pos;

		internal_server_request_parser::skip_ws(field_value, pos);
		++pos; // consume ';'
		internal_server_request_parser::skip_ws(field_value, pos);

		equal_sign_pos = field_value.find('=', pos);
		if (equal_sign_pos == std::string::npos) {
			log_error("RequestHeaderParser::parse_content_type",
					  std::string("equal sign is missing: '") + field_value.substr(pos) + "'");

			return Status::BadRequest();
		}
		if (!internal_server_request_parser::is_string_valid_token(field_value.c_str() + pos,
																   equal_sign_pos - pos)) {
			log_error("RequestHeaderParser::parse_content_type",
					  std::string("value is not a valid token: '") +
						  field_value.substr(pos, equal_sign_pos - pos) + "'");
			return Status::BadRequest();
		}

		std::string value;
		std::string parameter;
		parameter = field_value.substr(pos, equal_sign_pos - pos);
		pos = equal_sign_pos + 1;

		status = parse_parameter(field_value, value, pos, pos);
		if (!status || value.empty()) {
			log_error("RequestHeaderParser::parse_content_type",
					  std::string("failed to parse parameter: '") + field_value.substr(pos) + "'");
			return status;
		}
		media_type.parameters.insert(std::make_pair(parameter, value));
	}

	request.content_type = media_type;
	return Status::OK();
}

CGIResponseParser::FPtrFieldParser CGIResponseParser::get_field_parser_by_field_type(
	const std::string& field_type) {
	static std::map<const std::string, FPtrFieldParser> parsers;
	if (parsers.empty()) {
		parsers["status"] = &CGIResponseParser::parse_status;
		parsers["content-type"] = &CGIResponseParser::parse_content_type;
	}

	std::map<const std::string, FPtrFieldParser>::const_iterator it = parsers.find(field_type);
	if (it == parsers.end()) {
		return NULL;
	}

	return it->second;
}

Status CGIResponseParser::parse_header(t_request& request) {
	// const size_t len = content.size();
	const std::string header_end_key("\r\n\r\n");
	Status status;

	size_t header_end = _buffer.find(header_end_key);
	if (header_end == std::string::npos) {
		return Status::DataIsNotReady();
	}

	size_t pos = 0;

	// only two allowed headers are allowed: [Status, Content-Type]
	std::vector<std::string> headers;
	do {
		std::string field_type;
		std::string field_value;

		pos = internal_server_request_parser::get_token_with_delim(_buffer, pos, field_type, ": ",
																   true);
		pos = internal_server_request_parser::get_token_with_delim(_buffer, pos, field_value,
																   "\r\n", true);
		std::transform(field_type.begin(), field_type.end(), field_type.begin(),
					   static_cast<int (*)(int)>(std::tolower));
		FPtrFieldParser field_parser = get_field_parser_by_field_type(field_type);
		if (field_parser) {
			status = (this->*field_parser)(field_value, request);
			if (!status) {
				log_error("RequestHeaderParser::parse_header",
						  std::string("failed to parse field: '") + field_type +
							  "' with a value: '" + field_value + "'");
				return status;
			}
		}
	} while (pos < header_end);
	// pos += header_end_key.size();
	_header_parsed = true;
	_buffer = _buffer.substr(pos);

	return Status::OK();
}

void CGIResponseParser::log_error(const std::string& failed_component,
								  const std::string& message) const {
	if (_logger == NULL) {
		return;
	}

	_logger->log_error(failed_component, message);
}

Status CGIResponseParser::parse_body(t_request& request) {
	Status status;

	if (request.content_data.empty() == true) {
		request.content_data.push_back(t_request_content());
	}

	if (_buffer.empty() == false) {
		request.content_data.front().data.append(_buffer);
		_buffer.clear();
	}

	return Status::OK();
}

Status CGIResponseParser::parse(const std::string& content) {
	Status status;

	_buffer.append(content);
	if (_header_parsed == false) {
		status = parse_header(*_request);
		if (!status) {
			log_error("RequestHeaderParser::parse", std::string("failed to parse header'"));
			return Status::BadRequest(); //TODO
		}
	}	
	if (_header_parsed == true) {
		status = parse_body(*_request);
		if (!status) {
			log_error("RequestHeaderParser::parse", std::string("failed to parse body'"));
			return Status::BadRequest(); // TODO
		}
	}

	return Status::OK();
}
