#include "ServerResponse.hpp"

ServerResponse::ServerResponse(const t_request& request, const t_config& server_data)
	: _req_data(&request), _server_data(&server_data) {
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

ServerResponse& ServerResponse::serve_static_page(const t_location& loc, const std::string& uri) {
	std::string file_path = loc.common.root.empty() ? _server_data->common.root : loc.common.root;
	if (!file_path.empty() && file_path[file_path.size() - 1] != '/') file_path += "/";
	if (uri.length() >= loc.path.length()) {
		file_path += uri.substr(loc.path.length());
	}
	if (!file_path.empty() && file_path[file_path.size() - 1] == '/') {
		file_path += loc.common.index.empty() ? "index.html" : loc.common.index[0];
		const_cast<t_request*>(_req_data)->mime_type = ".html"; // maybe move it to parser?
	}
	_resp_content_type = identify_mime();
	header("content-type", _resp_content_type);
	if (is_binary()) {
		header("content-disposition", "inline");
		header("cache-control", "public, max-age=3600");
	}
	serve_file(file_path, false);
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
	if (_status.ok()) {
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

ServerResponse& ServerResponse::post_method() {
	_body += "post test";
	return *this;
}

ServerResponse& ServerResponse::delete_method() {
	_body += "delete test";
	return *this;
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
	int index = -1;
	size_t len = 0;
	for (size_t i = 0; i < _server_data->location.size(); ++i) {
		const std::string& location_path = _server_data->location[i].path;
		if (_req_data->uri_path.substr(0, location_path.length()) == location_path) {
			if (location_path.length() > len) {
				index = i;
				len = location_path.length();
				found = true;
			}
		}
	}
	if (found) {
		const t_location& loc = _server_data->location[index];
		if (_req_data->method == "POST" && loc.common.methods.postMethod)
			post_method();
		else if (_req_data->method == "GET" && loc.common.methods.getMethod)
			serve_static_page(loc, _req_data->uri_path);
		else if (_req_data->method == "DELETE" && loc.common.methods.deleteMethod)
			delete_method();
		else
			send_error_page(405, "Method Not Allowed");
	}
	if (!found) serve_default_root();
	header("server", _server_data->server_name[0]);
	header("content-length", get_body_size());
	_response = WS_PROTOCOL + _status.status_line() + get_headers() + "\r\n" + get_body();
	if (!is_binary()) std::cout << GREEN400 "RESPONSE:\n" << _response << RESET << std::endl;
	return _response;
}

ServerResponse& ServerResponse::json(const std::string& data) {
	_body = data;
	return *this;
}

void ServerResponse::serve_default_root() {
	if (_req_data->uri_path == "/") {
		t_location default_loc;
		default_loc.common.root = _server_data->common.root;
		default_loc.common.index = _server_data->common.index;
		default_loc.path = "/";
		serve_static_page(default_loc, "/");
	} else {
		send_error_page(404, "Not Found");
	}
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