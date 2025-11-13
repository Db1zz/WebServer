#ifndef SERVER_SERVER_REQUEST_PARSER
#define SERVER_SERVER_REQUEST_PARSER

#include <string>

#include "RequestHeaderParser.hpp"

class Status;
class ServerLogger;
class IRequestBodyParser;
typedef struct s_config t_config;
typedef struct s_request t_request;

class ServerRequestParser {
   public:
	explicit ServerRequestParser(const t_config* config, ServerLogger* logger = NULL);
	~ServerRequestParser();

	Status parse(const std::string& content, t_request& request);
	void reset();
	bool is_header_parsed() const;
	bool is_body_parsed() const;
	bool is_finished() const;

   private:
   Status parse_header(const std::string& content, size_t& body_pos_start, t_request& request);
   Status parse_body(const std::string& content, size_t body_pos_start, t_request& request);

	void create_body_parser(t_request& request);

   private:
	const t_config* _config;
	ServerLogger* _logger;

	RequestHeaderParser _header_parser;
	IRequestBodyParser* _body_parser;

	bool _header_parsed;
	bool _body_parsed;
};

#endif // SERVER_SERVER_REQUEST_PARSER