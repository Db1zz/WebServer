#ifndef SERVER_SERVER_UTILS_HPP_
#define SERVER_SERVER_UTILS_HPP_

#include <string>
#include <unistd.h>

#include "status.hpp"

typedef struct s_config t_config;

namespace server_utils {
void get_filename(const std::string& full_path, std::string& out);
void get_directory_path(const std::string& full_path, std::string& out);
void get_file_extension(const std::string& path, std::string& out);
bool get_cgi_bin(const std::string& uri_path, const t_config& server_config,
				 std::string& cgi_bin_out);
Status read_data(int fd, ssize_t read_buff_size, std::string& out_buffer, int& rd_bytes);
} // namespace server_utils

#endif // SERVER_SERVER_UTILS_HPP_