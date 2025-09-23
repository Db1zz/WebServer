#ifndef SERVER_REQUEST_PARSER_REQUEST_HEADER_PARSER_HPP_
#define SERVER_REQUEST_PARSER_REQUEST_HEADER_PARSER_HPP_

#include <string>

#include "ServerRequest.hpp"
#include "Utilities/status.hpp"

class RequestHeaderParser {
   public:
	RequestHeaderParser();
	Status feed(const std::string& content, size_t start_pos);
	Status apply(t_request& request);

   private:
	bool is_header_field_valid();

	Status parse_boundary(t_request& request);
	Status parse_request_line();
	bool is_method_valid(const std::string& method);
	Status get_filename_from_request_body(const std::string& request_string, std::string& result);
	Status get_mime_from_filename(const std::string& filename, std::string& result);
	bool is_header_complete();
	Status parse_complete_header(t_request& request);

	std::string _buffer;
};

#endif // SERVER_REQUEST_PARSER_REQUEST_HEADER_PARSER_HPP_