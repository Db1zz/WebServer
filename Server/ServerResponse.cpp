#include "ServerResponse.hpp"

ServerResponse::ServerResponse(ClientSocket* client_socket, const t_config& server_data)
	: _server_data(&server_data) {
	_req_data = client_socket ? &client_socket->get_connection_context()->request : NULL;
}

ServerResponse::~ServerResponse() {
}

ServerResponse& ServerResponse::operator<<(const std::string& data) {
	_body += data;
	return *this;
}

ServerResponse& ServerResponse::header(const std::string& key, const std::string& value) {
	_headers += key + ": " + value + "\r\n";
	return (*this);
}

ServerResponse& ServerResponse::serve_static_page(const t_location& loc) {
	struct stat path_stat;
	if (stat(_resolved_file_path.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
		if (loc.common.auto_index &&
			(_req_data->mime_type == ".json" || _req_data->accept == "*/*")) {
			json(_resolved_file_path);
			return *this;
		}
		if (!_resolved_file_path.empty() &&
			_resolved_file_path[_resolved_file_path.size() - 1] != '/')
			_resolved_file_path += "/";
		_resolved_file_path += loc.common.index.empty() ? "index.html" : loc.common.index[0];
		_req_data->mime_type = ".html";
	}

	_resp_content_type = identify_mime();
	header("content-type", _resp_content_type);
	if (is_binary()) {
		header("content-disposition", "inline");
		header("cache-control", "public, max-age=3600");
	}
	serve_file(_resolved_file_path, false);
	return *this;
}

bool ServerResponse::is_binary() {
	return (_req_data->mime_type == ".jpg" || _req_data->mime_type == ".jpeg" ||
			_req_data->mime_type == ".png" || _req_data->mime_type == ".gif" ||
			_req_data->mime_type == ".ico" || _req_data->mime_type == ".webp");
}

bool ServerResponse::serve_file(const std::string& path, bool is_error_page) {
	std::fstream file;
	_status = fs::open_file(file, path, std::ios::in | std::ios::binary);
	if (_status.is_ok()) {
		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		file.seekg(0, std::ios::beg);
		_body.resize(size);
		file.read(&_body[0], size);
		file.close();
		return true;
	} else if (!is_error_page) {
		_status.set_status_line(404, "Not Found");
		serve_file(_server_data->common.errorPage.at(404), true);
	} else
		return false;
	return false;
}

void ServerResponse::send_error_page(int code, std::string error_msg) {
	std::string path;
	_status.set_status_line(code, error_msg);
	std::stringstream code_str;
	code_str << code;
	header("content-type", "text/html");
	try {
		path = _server_data->common.errorPage.at(code);
	} catch (const std::out_of_range&) {
		path = "";
	}
	if (!serve_file(path, true)) {
		_status.set_status_line(code, error_msg);
		std::string err_msg = code_str.str() + " " + error_msg + " ";
		_body = "<!DOCTYPE html><html><head><title>" + err_msg +
				"</title></head>"
				"<body><h1>" +
				err_msg + "</h1></body></html>";
	}
}

std::string ServerResponse::generate_response() {
	_status.set_status_line(200, "OK");
	bool found = false;
	const t_location* best_match = NULL;
	size_t best_match_length = 0;

	for (size_t i = 0; i < _server_data->location.size(); ++i) {
		if (_req_data->uri_path.find(_server_data->location[i].path) == 0) {
			size_t location_length = _server_data->location[i].path.length();
			if (location_length > best_match_length) {
				best_match = &_server_data->location[i];
				best_match_length = location_length;
				found = true;
			}
		}
	}
	if (found && best_match) {
		resolve_file_path(*best_match);
		if (_req_data->method == "DELETE" && best_match->common.methods.deleteMethod)
			delete_method(*best_match);
		else if (_req_data->method == "GET" && best_match->common.methods.getMethod)
			serve_static_page(*best_match);
		else if (_req_data->method == "POST" && best_match->common.methods.postMethod)
			post_method(*best_match);
		else
			send_error_page(405, "Method Not Allowed");
	}
	if (!found) serve_default_root();
	header("server", _server_data->server_name[0]);
	header("content-length", get_body_size());
	_response = WS_PROTOCOL + _status.status_line() + get_headers() + "\r\n" + get_body();
	// std::cout << GREEN400 "RESPONSE:\n" << _response << RESET << std::endl;
	return _response;
}

ServerResponse& ServerResponse::json(const std::string& data) {
	if (data.empty())
		send_error_page(500, "Internal Server Error - No file management location found");
	DIR* dir = opendir(data.c_str());
	if (dir == NULL) std::string error_msg = "cannot access directory: " + data;
	std::string json_response = "[";
	struct dirent* entry;
	bool first = true;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
		if (!first) json_response += ",";
		json_response += "\"" + std::string(entry->d_name) + "\"";
		first = false;
	}

	json_response += "]";
	closedir(dir);
	_body = json_response;
	header("content-type", "application/json");
	header("access-control-allow-origin", "*");
	return *this;
}

void ServerResponse::serve_default_root() {
	if (_req_data->uri_path == "/") {
		t_location default_loc;
		default_loc.common.root = _server_data->common.root;
		default_loc.common.index = _server_data->common.index;
		default_loc.path = "/";
		resolve_file_path(default_loc);
		serve_static_page(default_loc);
	} else {
		send_error_page(404, "Not Found");
	}
}

ServerResponse& ServerResponse::handle_api_files() {
	std::string upload_directory = "";
	for (size_t i = 0; i < _server_data->location.size(); ++i) {
		const t_location& loc = _server_data->location[i];

		if (loc.common.methods.deleteMethod) {
			bool is_file_location = false;
			size_t dot_pos = loc.path.rfind('.');
			if (dot_pos != std::string::npos && dot_pos > loc.path.rfind('/'))
				is_file_location = true;

			if (!is_file_location) {
				std::string root_path =
					loc.common.root.empty() ? _server_data->common.root : loc.common.root;
				if (!root_path.empty() && root_path[root_path.size() - 1] != '/') root_path += "/";

				if (loc.path == "/")
					upload_directory = root_path;
				else
					upload_directory = root_path + loc.path.substr(1);
				break;
			}
		}
	}
	json(upload_directory);
	return *this;
}

void ServerResponse::resolve_file_path(const t_location& loc) {
	_resolved_file_path = loc.common.root.empty() ? _server_data->common.root : loc.common.root;
	if (!_resolved_file_path.empty() && _resolved_file_path[_resolved_file_path.size() - 1] != '/')
		_resolved_file_path += "/";

	std::string uri_path = _req_data->uri_path;
	if (!uri_path.empty() && uri_path[0] == '/') uri_path = uri_path.substr(1);
	_resolved_file_path += uri_path;
}

ServerResponse& ServerResponse::post_method(const t_location& loc) {
	if (!loc.common.methods.postMethod) {
		send_error_page(405, "Method Not Allowed");
		return *this;
	}

	std::string upload_dir = _resolved_file_path;
	bool file_saved = false;
	std::string file_path = upload_dir + _req_data->filename;
	struct stat file_stat;
	
	if (!upload_dir.empty() && upload_dir[upload_dir.size() - 1] != '/')
		upload_dir += "/";

	std::cout << CYAN200 <<  "Transferred length: " << _req_data->body_chunk.size() << std::endl;
	std::cout << CYAN200 <<  "Content-Length: " << _req_data->content_length << std::endl;
	if (stat(file_path.c_str(), &file_stat) == 0 && _req_data->transfered_length == 0) {
		_req_data->transfered_length = _req_data->content_length;
		std::cout << RED500 << "SENT A 409" << RESET<< std::endl;
		_status.set_status_line(409, "Conflict");
		_body = "{\"success\": false, \"message\": \"File already exists\"}";
		header("content-type", "application/json");
		return *this;
	}
	std::ofstream outfile(file_path.c_str(), std::ios::app | std::ios::binary);
	if (outfile) {
		outfile.write(_req_data->body_chunk.c_str(), _req_data->body_chunk.size());
		if (_req_data->is_request_ready()) {
			file_saved = true;
			outfile.close();
		}
	}
	if (file_saved) {
		_status.set_status_line(200, "OK");
		_body = "{\"success\": true, \"message\": \"Upload successful\"}";
		header("content-type", "application/json");
	} else if (!file_saved && _req_data->is_request_ready()) {
		_status.set_status_line(400, "Bad Request");
		_body = "{\"success\": false, \"message\": \"No file uploaded or failed to save file(s)\"}";
		header("content-type", "application/json");
	}
	else {
		_status.set_status_line(100, "Continue");
	}
	return *this;
}


ServerResponse& ServerResponse::delete_method(const t_location& loc) {
	if (!loc.common.methods.deleteMethod) {
		_status.set_status_line(405, "Method Not Allowed");
		_body =
			"{\"success\": false, \"message\": \"DELETE method not allowed for this location\"}";
	} else {
		if (access(_resolved_file_path.c_str(), F_OK) == 0) {
			if (unlink(_resolved_file_path.c_str()) == 0) {
				_status.set_status_line(200, "OK");
				_body = "{\"success\": true, \"message\": \"File deleted successfully\"}";
			} else {
				_status.set_status_line(500, "Internal Server Error");
				_body = "{\"success\": false, \"message\": \"Failed to delete file\"}";
			}
		} else {
			_status.set_status_line(404, "Not Found");
			_body = "{\"success\": false, \"message\": \"File not found\"}";
		}
	}
	header("content-type", "application/json");
	header("access-control-allow-origin", "*");
	return *this;
}

std::string ServerResponse::identify_mime() {
	if (_req_data->mime_type == ".html" || _req_data->mime_type == "") {
		_resp_content_type = "text/html";
	} else if (_req_data->mime_type == ".css") {
		_resp_content_type = "text/css";
	} else if (_req_data->mime_type == ".js") {
		_resp_content_type = "application/javascript";
	} else if (_req_data->mime_type == ".json") {
		_resp_content_type = "application/json";
	} else if (_req_data->mime_type == ".jpg" || _req_data->mime_type == ".jpeg") {
		_resp_content_type = "image/jpeg";
	} else if (_req_data->mime_type == ".png") {
		_resp_content_type = "image/png";
	} else if (_req_data->mime_type == ".gif") {
		_resp_content_type = "image/gif";
	} else if (_req_data->mime_type == ".svg") {
		_resp_content_type = "image/svg+xml";
	} else if (_req_data->mime_type == ".ico") {
		_resp_content_type = "image/x-icon";
	} else if (_req_data->mime_type == ".webp") {
		_resp_content_type = "image/webp";
	} else if (_req_data->mime_type == ".pdf") {
		_resp_content_type = "application/pdf";
	} else if (_req_data->mime_type == ".txt") {
		_resp_content_type = "text/plain";
	} else if (_req_data->mime_type == ".xml") {
		_resp_content_type = "application/xml";
	} else {
		_resp_content_type = "application/octet-stream";
	}
	return _resp_content_type;
}

const std::string ServerResponse::get_body_size() const {
	std::stringstream ss;
	ss << _body.size();
	return ss.str();
}

const std::string& ServerResponse::get_headers() const {
	return _headers;
}

const std::string& ServerResponse::get_body() const {
	return _body;
}