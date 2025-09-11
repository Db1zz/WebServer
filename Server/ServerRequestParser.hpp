#ifndef SERVER_SERVER_REQUEST_PARSER
#define SERVER_SERVER_REQUEST_PARSER

#include <string>

#include "ServerLogger.hpp"
#include "ServerRequest.hpp"
#include "ServerSocketManager.hpp"
#include "status.hpp"

/*
	HTTP 1.1 Requirements

	An implementation is not compliant if it fails to satisfy one or more
	of the MUST or REQUIRED level requirements for the protocols it
	implements. An implementation that satisfies all the MUST or REQUIRED
	level and all the SHOULD level requirements for its protocols is said
	to be "unconditionally compliant"; one that satisfies all the MUST
	level requirements but not all the SHOULD level requirements for its
	protocols is said to be "conditionally compliant."

	https://www.rfc-editor.org/rfc/rfc2616
*/

class ServerRequestParser {
   public:
	static Status parse_request_header(std::string& request_string, t_request& request,
									   ServerLogger& server_logger);
	static Status parse_request_body_chunk(t_request& request);
	static std::string& get_token(std::string& request_string, std::string& result,
								  const char* delims);

   private:
	// Deleted functions
	ServerRequestParser();
	ServerRequestParser(const ServerRequestParser& copy);
	ServerRequestParser& operator=(const ServerRequestParser& copy);

   protected:
	// Functions for internal use
	static bool is_method_valid(const std::string& method);
	// static Status normalize_uri(const t_commonConfig& common_server_config, std::string& uri);
	static bool is_unreserved(char c);
	static bool is_sub_delims(char c);
	static bool is_path_valid(const std::string& path);
	static bool is_pos_hex(const std::string& str, size_t pos);
	static std::string& consume_char(std::string& request_string, char c);
	static Status get_request_line(std::string& request_string, t_request& request);
	static Status get_request_headers(std::string& request_string, t_request& request);
	static Status extract_file_name_with_mime(const std::string& uri_path, std::string& filename,
											  std::string& mime_type);
	static Status get_mime_from_filename(const std::string& filename, std::string& result);
	static Status get_filename_from_request_body(const std::string& request_string,
												 std::string& result);
	static Status extract_request_body_header(t_request& request, std::string& result);
	static Status erase_request_body_end_boundary(t_request& request);
	static Status get_boundary(const std::string& request_string, t_request& request);
};

#endif // SERVER_SERVER_REQUEST_PARSER