#ifndef SERVER_REQUEST_PARSER_REQUEST_RAW_BODY_PARSER_HPP_
#define SERVER_REQUEST_PARSER_REQUEST_RAW_BODY_PARSER_HPP_

#include "IRequestBodyParser.hpp"

class RequestRawBodyParser : public IRequestBodyParser {
   public:
	RequestRawBodyParser(int content_length);
	Status feed(const std::string& content, size_t start_pos);
	void apply(t_request& request);

   private:
	std::string _data;
	int _data_size;
	const int _content_length;
};

#endif // SERVER_REQUEST_PARSER_REQUEST_RAW_BODY_PARSER_HPP_