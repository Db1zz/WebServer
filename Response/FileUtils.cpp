#include "FileUtils.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

FileUtils::FileUtils(t_request* req_data, const t_config* server_data)
	: _req_data(req_data), _server_data(server_data) {
}

FileUtils::~FileUtils() {
}

bool FileUtils::is_directory(const std::string& path) {
	struct stat path_stat;
	if (stat(path.c_str(), &path_stat) != 0) {
		return false;
	}
	return S_ISDIR(path_stat.st_mode);
}

void FileUtils::ensureTrailingSlash(std::string& path) {
	if (!path.empty() && path[path.size() - 1] != '/') {
		path += "/";
	}
}

bool FileUtils::is_file_exists(const std::string& file_path) {
	struct stat file_stat;
	return (stat(file_path.c_str(), &file_stat) == 0);
}

bool FileUtils::read_file_content(std::fstream& file, std::string& body) {
	if (!file.is_open() || !file.good())
		return false;
	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();

	if (size == -1) {
		file.close();
		return false;
	}
	file.seekg(0, std::ios::beg);

	body.resize(static_cast<size_t>(size));
	file.read(&body[0], size);
	file.close();

	return true;
}

std::string FileUtils::find_upload_directory() const {
	for (size_t i = 0; i < _server_data->location.size(); ++i) {
		const t_location& location = _server_data->location[i];

		if (location.common.methods.deleteMethod && is_file_location(location)) {
			std::string root_path =
				location.common.root.empty() ? _server_data->common.root : location.common.root;

			ensureTrailingSlash(root_path);

			if (location.path == "/") {
				return root_path;
			} else {
				return root_path + location.path.substr(1);
			}
		}
	}

	return "";
}

bool FileUtils::is_file_location(const t_location& location) const {
	size_t dot_pos = location.path.rfind('.');
	if (dot_pos == std::string::npos) {
		return false;
	}

	size_t last_slash_pos = location.path.rfind('/');
	return (dot_pos > last_slash_pos);
}

bool FileUtils::save_uploaded_file(const std::string& file_path, std::string &content_data) {
	std::ofstream outfile(file_path.c_str(), std::ios::app | std::ios::binary);

	if (!outfile) {
		return false;
	}

	outfile.write(content_data.c_str(), content_data.size());

	if (_req_data->is_request_ready()) {
		outfile.close();
		return true;
	}

	return false;
}

bool FileUtils::is_binary(const std::string& mime_type) const {
	return (mime_type == ".jpg" || mime_type == ".jpeg" || mime_type == ".png" ||
			mime_type == ".gif" || mime_type == ".ico" || mime_type == ".webp");
}

const t_location* FileUtils::find_best_location_match() const {
	const t_location* best_match = NULL;
	size_t best_match_length = 0;

	for (size_t i = 0; i < _server_data->location.size(); ++i) {
		const t_location& current_location = _server_data->location[i];

		if (_req_data->uri_path.find(current_location.path) == 0) {
			size_t location_length = current_location.path.length();

			if (location_length > best_match_length) {
				best_match = &current_location;
				best_match_length = location_length;
			}
		}
	}

	return best_match;
}

void FileUtils::resolve_file_path(const t_location& location, std::string& resolved_path) {
	resolved_path = location.common.root.empty() ? _server_data->common.root : location.common.root;

	ensureTrailingSlash(resolved_path);

	std::string uri_path = _req_data->uri_path;
	if (!uri_path.empty() && uri_path[0] == '/') {
		uri_path = uri_path.substr(1);
	}

	resolved_path += uri_path;
}

std::string FileUtils::identify_mime_type() const {
	std::string resp_content_type;

	if (_req_data->mime_type == ".html" || _req_data->mime_type == "") {
		resp_content_type = "text/html";
	} else if (_req_data->mime_type == ".css") {
		resp_content_type = "text/css";
	} else if (_req_data->mime_type == ".js") {
		resp_content_type = "application/javascript";
	} else if (_req_data->mime_type == ".json") {
		resp_content_type = "application/json";
	} else if (_req_data->mime_type == ".jpg" || _req_data->mime_type == ".jpeg") {
		resp_content_type = "image/jpeg";
	} else if (_req_data->mime_type == ".png") {
		resp_content_type = "image/png";
	} else if (_req_data->mime_type == ".gif") {
		resp_content_type = "image/gif";
	} else if (_req_data->mime_type == ".svg") {
		resp_content_type = "image/svg+xml";
	} else if (_req_data->mime_type == ".ico") {
		resp_content_type = "image/x-icon";
	} else if (_req_data->mime_type == ".webp") {
		resp_content_type = "image/webp";
	} else if (_req_data->mime_type == ".pdf") {
		resp_content_type = "application/pdf";
	} else if (_req_data->mime_type == ".txt") {
		resp_content_type = "text/plain";
	} else if (_req_data->mime_type == ".xml") {
		resp_content_type = "application/xml";
	} else {
		resp_content_type = "application/octet-stream";
	}
	return resp_content_type;
}