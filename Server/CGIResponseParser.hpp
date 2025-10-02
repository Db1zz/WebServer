#ifndef SERVER_CGI_RESPONSE_PARSER_HPP_
#define SERVER_CGI_RESPONSE_PARSER_HPP_

#include <string>

#include "ServerRequest.hpp"
#include "status.hpp"

class ServerLogger;
class CGIResponseParser {
   public:
	CGIResponseParser(t_request* request, ServerLogger* logger = NULL);

	typedef Status (CGIResponseParser::*FPtrFieldParser)(const std::string&, t_request&);
	FPtrFieldParser get_field_parser_by_field_type(const std::string& field_type);

	Status parse(const std::string& content);

   private:
	Status parse_parameter(const std::string& field_value, std::string& out, size_t pos,
						   size_t& separator_pos);
	Status parse_status(const std::string& field_valuem, t_request& request);
	Status parse_media_type(const std::string& field_value, t_media_type& media_type, size_t pos,
							size_t& separator_pos);
	Status parse_content_type(const std::string& field_value, t_request& request);

	bool is_valid_status_code(const std::string& status_str);
	void log_error(const std::string& failed_component, const std::string& message) const;
	Status parse_header(const std::string& content, t_request& request);
	Status parse_body(const std::string& content, t_request& request);

	std::string _buffer;
	bool _header_parsed;
	ServerLogger* _logger;
	t_request* _request;
};

#endif // SERVER_CGI_RESPONSE_PARSER_HPP_