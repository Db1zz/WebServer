#include "CGIResponseParser.hpp"

#include <algorithm>

#include "RequestParser/ServerRequestParserHelpers.hpp"
#include "ServerLogger.hpp"

CGIResponseParser::CGIResponseParser() : _header_found(false) {
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
		--pos; // do not eat separator.
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

	skip_ws(field_value, pos);
	while (pos < len && field_value[pos] == ';') {
		size_t equal_sign_pos;

		skip_ws(field_value, pos);
		++pos; // consume ';'
		skip_ws(field_value, pos);

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
		parser["status"] = parsers["content-type"] = &CGIResponseParser::parse_content_type;
	}

	std::map<const std::string, FPtrFieldParser>::const_iterator it = parsers.find(field_type);
	if (it == parsers.end()) {
		return NULL;
	}

	return it->second;
}

Status CGIResponseParser::parse_header(const std::string& content) {
	const std::string header_end_key("\r\n\r\n");
	size_t header_end = 0;

	_buffer.append(content);
	header_end = _buffer.find(header_end_key);
	if (header_end == std::string::npos) {
		return Status::DataIsNotReady();
	}

	size_t pos = 0;

	// there are only two allowed headers: [Status, Content-Type]
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

	} while (pos < header_end);

	return Status::OK();
}

void CGIResponseParser::log_error(const std::string& failed_component,
								  const std::string& message) const {
	if (_logger == NULL) {
		return;
	}

	_logger->log_error(failed_component, message);
}

// Status CGIResponseParser::parse(const std::string& content, size_t& body_start_pos) {
// }
