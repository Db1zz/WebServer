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

bool is_string_valid_token(const char* c);
bool is_string_valid_token(const char* c, size_t len);
bool is_char_tspecial(unsigned char c);
bool is_char_ctl(unsigned char c);
bool is_vchar(unsigned char c);
bool is_obs_text(unsigned char c);
bool is_ws(unsigned char c);
bool is_crlf(const char* c);
bool is_ows(const char* c);
bool is_qd_text(unsigned char c);

Status parse_nonencoded_filename(const std::string& filename);
Status parse_encoded_filename(const std::string& encoded_filename, std::string& decoded_filename);
Status decode_filename(const std::string& charset,
					   const std::string& language, const std::string& encoded_filename, std::string& decoded_filename);
Status utf8_char_decoder(const std::string& string, size_t char_pos, std::string& decoded, size_t& consumed_chars);
size_t char_to_hex(char c);
// TODO:
// Status normalize_uri(const t_commonConfig& common_server_config, std::string& uri);

} // namespace internal_server_request_parser

#endif // WEBSERVER_SERVER_SERVER_REQUEST_PARSER_HELPERS_HPP_