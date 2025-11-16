#include "RequestHeaderParser.hpp"

#include <stdint.h>

#include <algorithm>
#include <limits>
#include <map>
#include <sstream>
#include <utility>
#include <vector>

#include "ServerRequest.hpp"
#include "ServerRequestParserHelpers.hpp"
#include "ServerUtils.hpp"

RequestHeaderParser::RequestHeaderParser(const t_config* server_config, ServerLogger* logger)
	: _end_found(false), _server_config(server_config), _logger(logger) {
}

Status RequestHeaderParser::feed(const std::string& content, size_t& body_start_pos) {
	static const std::string header_end_key("\r\n\r\n");
	if (_end_found) {
		return Status::OK();
	}

	std::string temp_buffer = _buffer + content;
	size_t curr_end_pos = 0;

	curr_end_pos = temp_buffer.find(header_end_key);
	if (curr_end_pos == std::string::npos) {
		_buffer = temp_buffer;
		body_start_pos = curr_end_pos;
		return Status::DataIsNotReady();
	}

	// buff(abcd) - 4 | cs(efg) - 3 | tl(abcdefg) - 7 | s(de) - 2| (f) - 3 | (l) - 2
	// cs - (tl - (f + l)) = 1 | 3 - (7 - (3 + 2)) = 1
	int content_new_pos =
		content.size() - temp_buffer.size() + curr_end_pos + header_end_key.size();
	body_start_pos = content_new_pos < 0 ? 0 : content_new_pos;

	_buffer = temp_buffer.substr(0, curr_end_pos);
	_end_found = true;

	return Status::OK();
}

Status RequestHeaderParser::apply(t_request& request) {
	return parse_complete_header(request);
}

template <typename UINT>
static bool parse_uint(const std::string& s, UINT& out) {
	if (s.empty()) {
		return false;
	}

	UINT val = 0;
	for (size_t i = 0; i < s.size(); ++i) {
		char c = s[i];
		if (c < '0' || c > '9') {
			return false;
		}
		unsigned int digit = (unsigned int) (c - '0');
		if (val > (std::numeric_limits<UINT>::max() - digit) / 10) {
			return false;
		}
		val = val * 10 + digit;
	}
	out = val;
	return true;
}

/*
	Syntax:

	Content-Length-Header = "Content-Length" ":" OWS Content-Length OWS
	Content-Length = 1*DIGIT
*/
Status RequestHeaderParser::parse_content_length(const std::string& field_value,
												 t_request& request) {
	if (field_value.empty()) {
		log_error("RequestHeaderParser::parse_content_length", std::string("value is empty"));
		return Status::BadRequest();
	}

	const size_t len = field_value.size();
	size_t pos = 0;
	size_t start;
	std::string number;

	internal_server_request_parser::skip_ws(field_value, pos);

	start = pos;

	do {
		if (!std::isdigit(field_value[pos])) {
			log_error("RequestHeaderParser::parse_content_length",
					  std::string("char is not digit: '") + field_value[pos] + "'");
			return Status::BadRequest();
		}
		++pos;
	} while (pos < len);

	number = field_value.substr(start, pos);
	if (!parse_uint<uint64_t>(number, request.content_length)) {
		log_error("RequestHeaderParser::parse_content_length",
				  std::string("failed to convert string to uint: '") + number + "'");
		return Status::BadRequest();
	}

	return Status::OK();
}

static bool is_octet_valid(const std::string& octet) {
	const size_t len = octet.size();

	if (octet.empty()) {
		return false;
	}

	for (size_t i = 0; i < len; ++i) {
		if (!std::isdigit(octet[i])) {
			return false;
		}
	}

	int v = std::atoi(octet.c_str());

	return v >= 0 && v <= 255;
}

bool RequestHeaderParser::is_valid_ipv4_addr(const std::string& addr) {
	const size_t len = addr.size();
	const size_t octet_max_size = 3;
	size_t pos = 0;
	size_t octet_count = 0;

	// 0-255 . 0-255 . 0-255 . 0-255
	while (pos < len && octet_count != 4) {
		std::string octet;
		for (size_t i = 0; pos + i < len && i < octet_max_size; ++i) {
			if (addr[pos + i] == '.' || addr[pos + i] == ':') {
				break;
			}
			octet.push_back(addr[pos + i]);
		}

		if (octet.empty()) {
			return false;
		}

		if (!is_octet_valid(octet)) {
			return false;
		}

		pos += octet.size();
		++octet_count;

		if (octet_count == 4) {
			break;
		} else if (addr[pos] != '.') {
			return false;
		}
		++pos;
	}

	if (addr[pos] == ':') {
		std::string port_str;
		uint32_t port = 0;

		++pos; // skip ':'
		port_str = addr.substr(pos);
		if (!parse_uint<uint32_t>(port_str, port) || port > 65535) {
			return false;
		}
		pos += port_str.size();
	}
	if (pos < len || octet_count != 4) {
		return false;
	}
	return true;
}

// TODO: add domains support like example.com
Status RequestHeaderParser::parse_host(const std::string& field_value, t_request& request) {
	// if (!is_valid_ipv4_addr(field_value)) {
	// 	log_error("RequestHeaderParser::parse_host",
	// 			  std::string("invalid host: '") + field_value + "'");
	// 	return Status::BadRequest();
	// }

	request.host = field_value;

	return Status::OK();
}

RequestHeaderParser::FPtrFieldParser RequestHeaderParser::get_field_parser_by_field_type(
	const std::string& field_type) {
	static std::map<const std::string, FPtrFieldParser> parsers;
	if (parsers.empty()) {
		parsers["host"] = &RequestHeaderParser::parse_host;
		parsers["content-length"] = &RequestHeaderParser::parse_content_length;
		parsers["user-agent"] = &RequestHeaderParser::parse_user_agent;
		parsers["accept"] = &RequestHeaderParser::parse_accept;
		parsers["accept-language"] = &RequestHeaderParser::parse_accept_language;
		parsers["connection"] = &RequestHeaderParser::parse_connection;
		parsers["content-type"] = &RequestHeaderParser::parse_content_type;
		parsers["transfer-encoding"] = &RequestHeaderParser::parse_transfer_encoding;
		parsers["cookie"] = &RequestHeaderParser::parse_cookie;
	}

	std::map<const std::string, FPtrFieldParser>::const_iterator it = parsers.find(field_type);
	if (it == parsers.end()) {
		return NULL;
	}

	return it->second;
}

Status RequestHeaderParser::parse_user_agent(const std::string& field_value, t_request& request) {
	if (field_value.empty()) {
		log_error("RequestHeaderParser::parse_user_agent", std::string("field is empty"));
		return Status::BadRequest();
	}

	request.user_agent = field_value;
	return Status::OK();
}

Status RequestHeaderParser::parse_media_type(const std::string& field_value,
											 t_media_type& media_type, size_t pos,
											 size_t& separator_pos) {
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
		  type == "application" || type == "multipart" || type == "message" || type == "font" || type == "plain")) {
		// */* == any
		type = "*";
		subtype = "*";
	}

	media_type.type = type;
	media_type.subtype = subtype;
	separator_pos = pos;
	return Status::OK();
}

Status RequestHeaderParser::parse_parameter(const std::string& field_value, std::string& out,
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

Status RequestHeaderParser::parse_qvalue(const std::string& field_value, double& out, size_t pos,
										 size_t& separator_pos) {
	std::string number;
	double value;

	pos = internal_server_request_parser::get_token_with_delims(field_value, pos, number, ";,",
																false);
	if (number.size() > 5) {
		log_error("RequestHeaderParser::parse_qvalue",
				  std::string("invalid number format: '") + number + "'");

		return Status::BadRequest();
	}

	std::istringstream iss(number);
	iss >> value;

	if (iss.fail()) {
		log_error("RequestHeaderParser::parse_qvalue",
				  std::string("invalid number format: '") + number + "'");
		return Status::BadRequest();
	}

	out = value;
	separator_pos = pos;

	return Status::OK();
}

void RequestHeaderParser::skip_q_value(const std::string& field_value, size_t pos,
									   size_t& next_pos) {
	pos = field_value.find_first_of(";,", pos);
	if (pos == std::string::npos) {
		pos = field_value.size();
	}
	next_pos = pos;
}

Status RequestHeaderParser::parse_parameters(const std::string& field_value, t_accept_item& item,
											 size_t pos, size_t& separator_pos) {
	Status status;
	bool q_parsed = false;

	internal_server_request_parser::skip_ws(field_value, pos);
	// "\t;  q=0.123 \t ;  foo = \"bar baz\"  ;flag\t"
	while (field_value[pos] == ';') {
		size_t equal_sign_pos;

		internal_server_request_parser::skip_ws(field_value, pos);
		++pos; // consume ';'
		internal_server_request_parser::skip_ws(field_value, pos);

		equal_sign_pos = field_value.find('=', pos);
		if (equal_sign_pos == std::string::npos) {
			log_error("RequestHeaderParser::parse_parameters",
					  std::string("equal sign is missing: '") + field_value.substr(pos) + "'");

			return Status::BadRequest();
		}
		if (!internal_server_request_parser::is_string_valid_token(field_value.c_str() + pos,
																   equal_sign_pos - pos)) {
			log_error("RequestHeaderParser::parse_parameters",
					  std::string("value is not a valid token: '") +
						  field_value.substr(pos, equal_sign_pos - pos) + "'");
			return Status::BadRequest();
		}

		std::string parameter;
		parameter = field_value.substr(pos, equal_sign_pos - pos);
		pos = equal_sign_pos + 1;

		if (parameter != "q") {
			std::string value;
			status = parse_parameter(field_value, value, pos, pos);
			if (!status || value.empty()) {
				log_error(
					"RequestHeaderParser::parse_parameters",
					std::string("failed to parse parameter: '") + field_value.substr(pos) + "'");
				return status;
			}

			if (q_parsed) {
				item.accept_ext.push_back(std::make_pair(parameter, value));
			} else {
				item.media_type.parameters.insert(std::make_pair(parameter, value));
			}
		} else {
			if (q_parsed) {
				skip_q_value(field_value, pos, pos);
				continue;
			}

			status = parse_qvalue(field_value, item.q, pos, pos);
			if (!status) {
				log_error("RequestHeaderParser::parse_parameters",
						  std::string("failed to parse qvalue: '") + field_value.substr(pos) + "'");
				return status;
			}
			q_parsed = true;
		}
	}
	separator_pos = pos;
	return Status::OK();
}

// Accept         = #( media-range [ accept-params ] )

// media-range    = ( "*/*"
// 				/ ( type "/*" )
// 				/ ( type "/" subtype )
// 				) *( OWS ";" OWS parameter )

// accept-params  = weight *( accept-ext )
// weight         = OWS ";" OWS "q=" qvalue
// accept-ext     = OWS ";" OWS token [ "=" ( token / quoted-string ) ]

// type           = token
// subtype        = token
// parameter      = token "=" ( token / quoted-string )

// qvalue         = ( "0" [ "." 0*3DIGIT ] )
// 			/ ( "1" [ "." 0*3("0") ] )

// aboba example:
// image/avif,image/webp,image/png,image/svg+xml,image/*;q=0.8,*/*;q=0.5
// Accept: text/html, application/xhtml+xml, application/xml;q=0.9, */*;q=0.8
// Accept: audio/*; q=0.2, audio/basic
// Accept: text/html;charset=utf-8;q=0.7;level=1, text/html;q=0.7, text/*;q=0.5

// parse type/subtype
//		parse parameters

// parameter      = token "=" ( token / quoted-string )
Status RequestHeaderParser::parse_accept(const std::string& field_value, t_request& request) {
	const size_t len = field_value.size();
	Status status;
	std::vector<t_accept_item> accept_items;
	size_t pos = 0;

	if (field_value.empty()) {
		log_error("RequestHeaderParser::parse_accept", std::string("value is empty"));
		return Status::BadRequest();
	}

	while (pos < len) {
		t_accept_item item;
		status = parse_media_type(field_value, item.media_type, pos, pos);
		if (!status) {
			log_error("RequestHeaderParser::parse_accept",
					  std::string("failed to parse media type: '") + field_value.substr(pos) + "'");
			return status;
		}
		status = parse_parameters(field_value, item, pos, pos);
		if (!status) {
			log_error("RequestHeaderParser::parse_accept",
					  std::string("failed to parse parameters: '") + field_value.substr(pos) + "'");
			return status;
		}
		accept_items.push_back(item);
		if (field_value[pos] == ',') {
			++pos; // consume ','
		}
	}

	request.accept = field_value; // LMAO it's made on purpose, trust me :)
	request.accept_items = accept_items;
	return Status::OK();
}

// TODO
Status RequestHeaderParser::parse_accept_language(const std::string& field_value,
												  t_request& request) {
	if (field_value.empty()) {
		log_error("RequestHeaderParser::parse_accept_language", std::string("value is empty"));
		return Status::BadRequest();
	}

	request.language = field_value;
	return Status::OK();
}

Status RequestHeaderParser::parse_connection(const std::string& field_value, t_request& request) {
	if (field_value.empty()) {
		log_error("RequestHeaderParser::parse_connection", std::string("value is empty"));
		return Status::BadRequest();
	}

	size_t pos = 0;

	internal_server_request_parser::skip_ws(field_value, pos);
	if (!internal_server_request_parser::is_string_valid_token(field_value.c_str() + pos,
															   field_value.size() - pos)) {
		log_error("RequestHeaderParser::parse_connection",
				  std::string("value is not a valid token: '") + field_value.substr(pos) + "'");
		return Status::BadRequest();
	}

	std::string lower_field_value(field_value);
	std::transform(lower_field_value.begin(), lower_field_value.end(), lower_field_value.begin(),
				   static_cast<int (*)(int)>(std::tolower));

	if (lower_field_value != "keep-alive" && lower_field_value != "transfer-encoding" &&
		lower_field_value != "close") {
		log_error("RequestHeaderParser::parse_connection",
				  std::string("unkown field: '") + field_value.substr(pos) + "'");
		return Status::BadRequest();
	}

	request.connection = lower_field_value;
	return Status::OK();
}

bool RequestHeaderParser::is_content_type_valid(t_request& request) {
	if (request.content_type.type == "multipart" && request.content_type.subtype == "form-data") {
		const std::string* boundary = request.content_type.find_parameter("boundary");
		if (boundary == NULL) {
			log_error("RequestHeaderParser::is_content_type_valid",
					  std::string("boundary was not provided"));
			return false;
		}
	}
	return true;
}

Status RequestHeaderParser::is_request_valid(t_request& request) {
	// if (request.host.empty()) {
	// 	log_error("RequestHeaderParser::is_request_valid", std::string("HOST was not provided"));
	// 	return false;
	// }

	// TODO: we can avoid content_length with POST method when chunked encoding is sent.
	if (request.method == "POST" && request.content_length == 0) {
		log_error("RequestHeaderParser::is_request_valid",
				  std::string("request method is 'POST' but content length was not provided."));
		return Status::BadRequest();
	}

	if (!is_content_type_valid(request)) {
		return Status::BadRequest();
	}

	size_t max_body_size = server_utils::get_location_max_body_size(request.uri_path, *_server_config);
	if (request.method == "POST" && max_body_size > 0 && max_body_size < request.content_length) {
		return Status::RequestEntityTooLarge();
	}

	return Status::OK();
}

Status RequestHeaderParser::parse_content_type(const std::string& field_value, t_request& request) {
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

Status RequestHeaderParser::parse_encoding_types(const std::string& field_value,
												 std::vector<std::string>& out) {
	if (field_value.empty()) {
		log_error("RequestHeaderParser::parse_encoding_types", std::string("value is empty"));
		return Status::BadRequest();
	}

	const size_t len = field_value.size();
	size_t pos = 0;
	std::vector<std::string> encoding_types;

	while (pos < len) {
		std::string type;
		pos =
			internal_server_request_parser::get_token_with_delim(field_value, pos, type, ",", true);
		if (type.empty()) {
			log_error("RequestHeaderParser::parse_encoding_types",
					  std::string("type is empty in this field value: '") + field_value + "'");
			return Status::BadRequest();
		}

		if (!internal_server_request_parser::is_string_valid_token(type.c_str())) {
			log_error("RequestHeaderParser::parse_encoding_types",
					  std::string("type '") + type + "' is not a valid token");
			return Status::BadRequest();
		}
		encoding_types.push_back(type);
	}

	out = encoding_types;

	return Status::OK();
}

Status RequestHeaderParser::parse_transfer_encoding(const std::string& field_value,
													t_request& request) {
	return parse_encoding_types(field_value, request.transfer_encoding);
}

Status RequestHeaderParser::parse_complete_header(t_request& request) {
	if (!_end_found) {
		return Status::DataIsNotReady();
	}

	const size_t len = _buffer.size();
	Status status;
	size_t pos = 0;
	std::string request_line;

	internal_server_request_parser::unfold_string(_buffer); // remove \r\n + ws

	pos = internal_server_request_parser::get_token_with_delim(_buffer, pos, request_line, "\r\n",
															   true);
	status = parse_request_line(request_line, request);
	if (!status) {
		log_error("RequestHeaderParser::parse_complete_header",
				  std::string("failed to parse request line: '") + _buffer.substr(pos) + "'");
		return status;
	}

	if (server_utils::get_cgi_bin(request.uri_path, *_server_config, request.cgi_bin)) {
		request.is_cgi = true;
	}

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
				log_error("RequestHeaderParser::parse_complete_header",
						  std::string("failed to parse field: '") + field_type +
							  "' with a value: '" + field_value + "'");
				return status;
			}
		}
	} while (pos < len);

	status = is_request_valid(request);
	return status;
}

Status RequestHeaderParser::parse_cookie(const std::string& field_value, t_request& request) {
	std::string::size_type pos = field_value.find("session_id=");
	if (pos == std::string::npos) {
		return Status::OK();
	}

	pos += std::string("session_id=").size();
	std::string::size_type end = field_value.find(';', pos);
	std::string value;
	if (end == std::string::npos) {
		value = field_value.substr(pos);
	} else {
		value = field_value.substr(pos, end - pos);
	}
	size_t start = 0;
	while (start < value.size() && isspace(static_cast<unsigned char>(value[start]))) ++start;
	size_t finish = value.size();
	while (finish > start && isspace(static_cast<unsigned char>(value[finish - 1]))) --finish;
	request.session_id = value.substr(start, finish - start);

	return Status::OK();
}

// request-line   = method SP request-target SP HTTP-version CRLF
// method         = token
// request-target = origin-form
//                / absolute-form
//                / authority-form
//                / asterisk-form
// HTTP-version   = "HTTP/" DIGIT "." DIGIT
Status RequestHeaderParser::parse_request_line(const std::string& request_line,
											   t_request& request) {
	Status status;
	size_t pos = 0;
	std::string method;
	std::string uri_path;
	std::string protocol_version;

	pos =
		internal_server_request_parser::get_token_with_delims(request_line, pos, method, " ", true);
	pos = internal_server_request_parser::get_token_with_delim(request_line, pos, uri_path, " ",
															   true);
	protocol_version = request_line.substr(pos);

	if (method.empty() || uri_path.empty() || protocol_version.empty()) {
		log_error("RequestHeaderParser::parse_request_line", std::string("invalid request line"));
		return Status::BadRequest();
	}

	if (!is_method_valid(method)) {
		log_error("RequestHeaderParser::parse_request_line",
				  std::string("method is not implemented: '") + request.method + "'");
		return Status::NotImplemented();
	}

	status = parse_absolute_path(uri_path, request);
	if (!status) {
		log_error("RequestHeaderParser::parse_request_line",
				  std::string("failed to parse absolute path: '") + uri_path + "'");
		return status;
	}

	if (!is_protocol_valid(protocol_version)) {
		log_error("RequestHeaderParser::parse_request_line", std::string("invalid request line"));
		return Status::BadRequest();
	}

	if (!is_supported_protocol_version(protocol_version)) {
		log_error("RequestHeaderParser::parse_request_line",
				  std::string("unsupported HTTP version: '") + protocol_version + "'");
		return Status::HTTPVersionNotSupported();
	}

	request.method = method;
	request.protocol_version = protocol_version;

	return Status::OK();
}

bool RequestHeaderParser::is_method_valid(const std::string& method) {
	return (method == "GET" || method == "POST" || method == "DELETE");
}

bool RequestHeaderParser::is_supported_protocol_version(const std::string& protocol_version) {
	return (protocol_version == "HTTP/1.1" || protocol_version == "HTTP/1.0");
}

bool RequestHeaderParser::is_protocol_valid(const std::string& protocol_version) {
	static const size_t expected_len = 8;
	static const std::string http_key("HTTP/");
	size_t pos;

	if (protocol_version.size() != expected_len) {
		return false;
	}

	pos = protocol_version.find(http_key);
	if (pos != 0) {
		return false;
	}

	pos += http_key.size();
	if (!std::isdigit(protocol_version[pos]) || protocol_version[pos + 1] != '.' ||
		!std::isdigit(protocol_version[pos + 2])) {
		return false;
	}

	return true;
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

Status RequestHeaderParser::parse_absolute_path_queries(const std::string& uri_path,
														std::vector<std::string>& out, size_t pos) {
	const size_t len = uri_path.size();
	Status status;
	std::vector<std::string> queries;

	while (pos < len) {
		std::string query;

		while (pos < len && uri_path[pos] != '&') {
			if (uri_path[pos] == '/' || uri_path[pos] == '?') {
				query.push_back(uri_path[pos]);
			} else if (internal_server_request_parser::is_pchar(uri_path[pos])) {
				query.push_back(uri_path[pos]);
			} else if (internal_server_request_parser::is_pos_hex(uri_path, pos)) {
				std::string decoded;
				size_t consumed;
				status = internal_server_request_parser::utf8_char_decoder(uri_path, pos, decoded,
																		   consumed);
				if (!status) {
					log_error("RequestHeaderParser::parse_absolute_path",
							  std::string("failed to decode UTF-8 encoded bytes: '") +
								  uri_path.substr(pos) + "'");
					return status;
				}

				pos += consumed;
				query.append(decoded);
				continue;
			} else {
				break;
			}
			++pos;
		}
		if (query.empty()) {
			log_error("RequestHeaderParser::parse_absolute_path", std::string("query is empty"));
			return Status::BadRequest();
		}

		queries.push_back(query);
		if (uri_path[pos] == '&') {
			++pos;
		}
	}

	out = queries;

	return status;
}

Status RequestHeaderParser::parse_absolute_path(const std::string& uri_path, t_request& request) {
	const size_t len = uri_path.size();
	Status status;
	size_t pos = 0;
	std::string path;

	if (len == 0 || uri_path[0] != '/') {
		log_error("RequestHeaderParser::parse_absolute_path",
				  std::string("the path must start with '/' character, but starts with: '") +
					  uri_path[0] + "'");
		return Status::BadRequest();
	}

	while (pos < len && uri_path[pos] != '?') {
		if (uri_path[pos] == '/') {
			path.push_back(uri_path[pos]);
		} else if (internal_server_request_parser::is_pchar(uri_path[pos])) {
			path.push_back(uri_path[pos]);
		} else if (internal_server_request_parser::is_pos_hex(uri_path, pos)) {
			std::string decoded;
			size_t consumed;
			status =
				internal_server_request_parser::utf8_char_decoder(uri_path, pos, decoded, consumed);
			if (!status) {
				log_error("RequestHeaderParser::parse_absolute_path",
						  std::string("failed to decode UTF-8 encoded bytes: '") +
							  uri_path.substr(pos) + "'");
				return status;
			}

			pos += consumed;
			path.append(decoded);
			continue;
		} else {
			break;
		}
		++pos;
	}

	if (pos < len && uri_path[pos] != '?' &&
		!internal_server_request_parser::is_pchar(uri_path[pos])) {
		log_error("RequestHeaderParser::parse_absolute_path",
				  std::string("invalid path format: '") + uri_path + "'");
		return Status::BadRequest();
	}

	if (uri_path[pos] == '?') {
		++pos;
		status = parse_absolute_path_queries(uri_path, request.path_queries, pos);
		if (!status) {
			log_error("RequestHeaderParser::parse_absolute_path",
					  std::string("failed to parse queries: '") + uri_path + "'");
			return Status::BadRequest();
		}
	}

	request.uri_path = path;
	server_utils::get_filename(request.uri_path, request.filename);
	internal_server_request_parser::extract_mime(request.filename, request.mime_type);
	return Status::OK();
}

void RequestHeaderParser::log_error(const std::string& failed_component,
									const std::string& message) const {
	if (_logger == NULL) {
		return;
	}

	_logger->log_error(failed_component, message);
}
