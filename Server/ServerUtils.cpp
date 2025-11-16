#include "ServerUtils.hpp"

#include <iostream>

#include "ServerConfig.hpp"

namespace server_utils {

// get_filename
void get_filename(const std::string& full_path, std::string& out) {
	size_t last_slash_pos = 0;

	last_slash_pos = full_path.find_last_of("/");
	if (last_slash_pos == std::string::npos) {
		return;
	}
	out = full_path.substr(last_slash_pos + 1);
}

// parses /cgi-bin/aboba.py and /cgi-bin/aboba.py/ as /cgi-bin
// the get_directory_path always removes trailing slash except the case
// when full_path == '/'
void get_directory_path(const std::string& full_path, std::string& out) {
	if (full_path.empty()) {
		out = "/";
		return;
	}

	if (full_path.size() == 1 && full_path[0] == '/') {
		out = full_path;
		return;
	}

	size_t end = full_path.size() - 1;
	end = full_path.rfind("/", end);
	if (end == std::string::npos) {
		out.clear();
		return;
	}
	if (end == 0) {
		out = "/";
	} else {
		out = full_path.substr(0, end);
	}
}

void get_file_extension(const std::string& path, std::string& out) {
	if (path.empty() || path.size() < 3) {
		out.clear();
		return;
	}

	size_t end = path.size() - 1;
	size_t start = end;
	if (path[end] == '/') {
		--end;
	}
	start = path.rfind('.', start);
	if (start == std::string::npos || end - start == 0) {
		out.clear();
		return;
	}

	out = path.substr(start, end);
}

bool get_cgi_bin(const std::string& uri_path, const t_config& server_config,
				 std::string& cgi_bin_out) {
	std::map<std::string, std::string>::const_iterator it;
	std::string file_extension;
	std::string dir_path;

	get_file_extension(uri_path, file_extension);
	get_directory_path(uri_path, dir_path);

	if (file_extension.empty() || dir_path.empty()) {
		return false;
	}

	if (dir_path == "/") {
		it = server_config.common.cgi.find(file_extension);
		if (it == server_config.common.cgi.end()) {
			cgi_bin_out.empty();
			return false;
		}
		cgi_bin_out = it->second;
		return true;
	}

	const std::vector<t_location>& locations = server_config.location;
	for (size_t i = 0; i < locations.size(); ++i) {
		if (locations[i].path != dir_path) {
			continue;
		}

		const std::map<std::string, std::string>& bin_map = locations[i].common.cgi;
		it = bin_map.find(file_extension);
		if (it == bin_map.end()) {
			cgi_bin_out.empty();
			return false;
		}
		cgi_bin_out = it->second;
		return true;
	}
	return false;
}

size_t get_location_max_body_size(const std::string& uri_path, const t_config& server_config) {
	std::map<std::string, std::string>::const_iterator it;
	std::string dir_path;

	get_directory_path(uri_path, dir_path);
	if (dir_path == "/") {
		return server_config.common.max_client_body;
	}

	const std::vector<t_location>& locations = server_config.location;
	for (size_t i = 0; i < locations.size(); ++i) {
		if (locations[i].path != dir_path) {
			continue;
		}
		return locations[i].common.max_client_body;
	}
	return 0;
}

Status read_data(int fd, ssize_t read_buff_size, std::string& buff, int& rd_bytes) {
	char read_buff[read_buff_size];

	rd_bytes = read(fd, read_buff, read_buff_size);
	//std::cout << "request: " << read_buff << std::endl;
	if (rd_bytes < 0) {
		return Status::InternalServerError();
	}
	buff.append(read_buff, rd_bytes);
	return Status::OK();
}

} // namespace server_utils