#ifndef SERVER_REQUEST_PARSER_IREQUEST_BODY_PARSER_HPP_
#define SERVER_REQUEST_PARSER_IREQUEST_BODY_PARSER_HPP_

#include <string>
#include "status.hpp"
#include "ServerRequest.hpp"

class IRequestBodyParser {
public:
	virtual ~IRequestBodyParser() {};
	virtual Status feed(const std::string& content, size_t start_pos) = 0;
	virtual void apply(t_request& request) = 0;
	virtual bool is_finished() const = 0;
};

#endif // SERVER_REQUEST_PARSER_IREQUEST_BODY_PARSER_HPP_