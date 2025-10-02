#ifndef SERVER_CGI_RESPONSE_PARSER_HPP_
#define SERVER_CGI_RESPONSE_PARSER_HPP_

#include <string>

#include "status.hpp"
#include "ServerRequest.hpp"


class ServerLogger;
class CGIResponseParser {
public:
	CGIResponseParser();

	typedef Status(CGIResponseParser::*FPtrFieldParser)(const std::string&, t_request&);
	FPtrFieldParser get_field_parser_by_field_type(const std::string& field_type);

	Status feed(const std::string& content, size_t& body_start_pos);
	Status apply(t_request& request);

   private:
	Status parse_status();
	Status parse_media_type(const std::string& field_value, t_media_type& media_type, size_t pos,
							size_t& separator_pos);
	Status parse_content_type(const std::string& field_value, t_request& request);

	void log_error(const std::string& failed_component, const std::string& message) const;

	std::string _buffer;
	bool _header_found;
   	Status parse_header(const std::string& content);
	ServerLogger* _logger;
};

#endif // SERVER_CGI_RESPONSE_PARSER_HPP_