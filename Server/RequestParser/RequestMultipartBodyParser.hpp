#ifndef SERVER_REQUEST_PARSER_REQUEST_MULTIPART_PARSER_HPP_
#define SERVER_REQUEST_PARSER_REQUEST_MULTIPART_PARSER_HPP_

#include <list>

#include "IRequestBodyParser.hpp"

#define MAX_REQUEST_BODY_HEADER_LENGTH 2048
#define MAX_FILENAME_SIZE 78
#define MIME_TYPE_MAX_SIZE 127
#define MIME_SUBTYPE_MAX_SIZE 127

class ServerLogger;

class RequestMultipartParser : public IRequestBodyParser {
   public:
	RequestMultipartParser(const std::string& boundary, int content_length, ServerLogger* server_logger = NULL);
	Status feed(const std::string& content, size_t start_pos);
	void apply(t_request& request);
	bool is_finished() const;

   private:
	size_t search_boundary();
	Status parse_body_without_any_boundary();
	Status parse_body_with_header(size_t boundary_pos);
	Status parse_and_finish_content(size_t boundary_pos);
	Status parse_body_header(size_t boundary_pos);
	Status parse_filename(const std::string& line, std::string& filename);
	Status parse_unquoted_filename(const std::string& line, size_t pos, std::string& filename);
	Status parse_name(const std::string& content_disposition, std::string& name);
	Status parse_content_type(const std::string& content_type, std::string& mime);

	void update_last_content();
	const int _content_length;

	std::string _buffer;
	int _data_size;

	std::string _start_boundary;
	std::string _end_boundary;

	std::list<t_request_content> _content_data;
	t_request_content* _last_content;
	bool _end_boundary_found;
	bool _finished;
	bool _first_header_found;

	ServerLogger* _server_logger;
};

#endif // SERVER_REQUEST_PARSER_REQUEST_MULTIPART_PARSER_HPP_