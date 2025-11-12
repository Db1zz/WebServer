#ifndef SERVER_REQUEST_PARSER_REQUEST_HEADER_PARSER_HPP_
#define SERVER_REQUEST_PARSER_REQUEST_HEADER_PARSER_HPP_

#include <string>

#include "ServerLogger.hpp"
#include "ServerRequest.hpp"
#include "ServerConfig.hpp"
#include "status.hpp"

class RequestHeaderParser {
   public:
	RequestHeaderParser(const t_config* server_config, ServerLogger* logger = NULL);
	Status feed(const std::string& content, size_t& body_start_pos);
	Status apply(t_request& request);

   private:
	Status parse_request_line(const std::string& request_line, t_request& request);
	bool is_method_valid(const std::string& method);
	Status get_mime_from_filename(const std::string& filename, std::string& result);
	Status parse_absolute_path_queries(const std::string& uri_path, std::vector<std::string>& out,
									   size_t pos);
	Status parse_absolute_path(const std::string& uri_path, t_request& request);
	Status parse_complete_header(t_request& request);

	// field parsers
	Status parse_cookie(const std::string& field_value, t_request& request);
	Status parse_host(const std::string& field_value, t_request& request);
	Status parse_content_length(const std::string& field_value, t_request& request);
	Status parse_user_agent(const std::string& field_value, t_request& request);
	Status parse_accept(const std::string& field_value, t_request& request);
	Status parse_accept_language(const std::string& field_value, t_request& request);
	Status parse_connection(const std::string& field_value, t_request& request);
	Status parse_content_type(const std::string& field_value, t_request& request);
	Status parse_encoding_types(const std::string& field_value, std::vector<std::string>& out);
	Status parse_transfer_encoding(const std::string& field_value, t_request& request);

	Status parse_media_type(const std::string& field_value, t_media_type& media_type, size_t pos,
							size_t& separator_pos);
	Status parse_parameter(const std::string& field_value, std::string& out, size_t pos,
						   size_t& separator_pos);
	Status parse_qvalue(const std::string& field_value, double& out, size_t pos,
						size_t& separator_pos);
	void skip_q_value(const std::string& field_value, size_t pos, size_t& next_pos);
	Status parse_parameters(const std::string& field_value, t_accept_item& item, size_t pos,
							size_t& separator_pos);
	bool is_valid_ipv4_addr(const std::string& addr);

	bool is_content_type_valid(t_request& request);
	bool is_request_valid(t_request& request);
	bool is_supported_protocol_version(const std::string& protocol_version);
	bool is_protocol_valid(const std::string& protocol_version);

	typedef Status (RequestHeaderParser::*FPtrFieldParser)(const std::string&, t_request&);
	FPtrFieldParser get_field_parser_by_field_type(const std::string& field_type);

	void log_error(const std::string& failed_component, const std::string& message) const;
	std::string _buffer;
	bool _end_found;
	const t_config* _server_config;
	ServerLogger* _logger;
};

#endif // SERVER_REQUEST_PARSER_REQUEST_HEADER_PARSER_HPP_