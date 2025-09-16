#ifndef SERVER_SERVER_REQUEST_PARSER
#define SERVER_SERVER_REQUEST_PARSER

#include <string>

class Status;
typedef struct s_request t_request;

class ServerRequestParser {
   public:
	// Public Functions
	explicit ServerRequestParser(t_request* request);
	ServerRequestParser(const ServerRequestParser& copy);
	ServerRequestParser& operator=(const ServerRequestParser& copy);

	Status parse_chunk(const std::string& chunk);

	// void reset();
	// void set_request(t_request* request);

   private:
	// Internal Functions
	bool is_method_valid(const std::string& method);

	Status parse_request_header();
	Status parse_request_line();
	Status parse_boundary();
	Status get_filename_from_request_body(const std::string& request_string, std::string& result);
	Status get_mime_from_filename(const std::string& filename, std::string& result);

   private:
	// Internal Variables
	t_request* _request;
	size_t _cursor_pos;

	bool _request_header_parsed;
	bool _boundary_header_parsed;

	std::string _boundary_start;
	std::string _boundary_end;

	std::string _cache;
};

#endif // SERVER_SERVER_REQUEST_PARSER