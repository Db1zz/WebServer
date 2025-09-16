#ifndef WEBSERVER_SERVER_SERVER_REQUEST_PARSER_HELPERS_HPP_
#define WEBSERVER_SERVER_SERVER_REQUEST_PARSER_HELPERS_HPP_

#include <string>

#include "status.hpp"

namespace internal_server_request_parser {

bool is_unreserved(char c);
bool is_sub_delims(char c);
bool is_path_valid(const std::string& path);
bool is_pos_hex(const std::string& str, size_t pos);

size_t get_token_with_delims(const std::string& data, size_t start, std::string& result,
							 const char* delims);
size_t get_token_with_delim(const std::string& data, size_t start, std::string& result,
							const char* delim);
std::string& consume_char(std::string& request_string, char c);

Status parse_file_name_with_mime(const std::string& uri_path, std::string& filename,
								 std::string& mime_type);
Status parse_request_body_header(const std::string& cache, size_t& pos, size_t& transfered_length,
								 const std::string& boundary_start, std::string& result);
Status parse_request_body_chunk(std::string& cache, size_t& pos, size_t& transfered_length,
								const std::string& boundary_end, std::string& result);

// TODO:
// Status normalize_uri(const t_commonConfig& common_server_config, std::string& uri);

} // namespace internal_server_request_parser

#endif // WEBSERVER_SERVER_SERVER_REQUEST_PARSER_HELPERS_HPP_