#ifndef SERVER_SERVER_REQUEST_PARSER
#define SERVER_SERVER_REQUEST_PARSER

#include <string>

#include "RequestHeaderParser.hpp"

class Status;
class ServerLogger;
class IRequestBodyParser;
typedef struct s_request t_request;

class ServerRequestParser {
   public:
	explicit ServerRequestParser(t_request* request, ServerLogger* logger = NULL);
	~ServerRequestParser();

	Status parse_header(const std::string& content);
	Status parse_body(const std::string& content);
	bool is_cgi_request() const;
	bool is_header_parsed() const;


   private:
	void create_body_parser();

   private:
	std::string _buffer;
	ServerLogger* _logger;

	RequestHeaderParser _header_parser;
	IRequestBodyParser* _body_parser;

	bool _is_cgi;
	bool _header_parsed;

	t_request* _request;
};

#endif // SERVER_SERVER_REQUEST_PARSER