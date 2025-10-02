#ifndef SERVER_CGI_RESPONSE_PARSER_HPP_
#define SERVER_CGI_RESPONSE_PARSER_HPP_

#include "status.hpp"
#include "ServerRequest.hpp"

class CGIResponseParser {
public:
	CGIResponseParser();

	Status feed(const std::string& content, size_t& body_start_pos);
	Status apply(t_request& request);

   private:
   	Status parse_header(const std::string& content);
};

#endif // SERVER_CGI_RESPONSE_PARSER_HPP_