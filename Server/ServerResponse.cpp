#include "ServerResponse.hpp"

ServerResponse::ServerResponse(const t_request& request,
							   const t_config& server_data)
	: _req_data(&request), _server_data(&server_data) {}

ServerResponse::~ServerResponse() {}

ServerResponse& ServerResponse::operator<<(const std::string& data) {
	_body += data;
	return *this;
}

ServerResponse& ServerResponse::header(const std::string& key,
									   const std::string& value) {
	_headers += key + ": " + value + "\r\n";
	return (*this);
}

ServerResponse& ServerResponse::serve_static_page(const t_location& loc,
												  const std::string& uri) {
	std::string file_path = loc.common.root + uri.substr(loc.path.length());
	if (!file_path.empty() && file_path[file_path.size() - 1] == '/') {
		file_path +=
			loc.common.index.empty() ? "index.html" : loc.common.index[0];
	}
	_resp_content_type = identify_mime();
	header("content-type", _resp_content_type);
	html(file_path, false);
	header("content-length", get_body_size());
	_response = WS_PROTOCOL + _status.status_line() + get_headers() + "\r\n" +
				get_body();
	return *this;
}

bool ServerResponse::html(const std::string& path, bool is_error_page) {
	std::fstream html_file;
	_status = fs::open_file(html_file, path, std::ios::in);
	if (_status.ok()) {
		std::string temp;
		while (getline(html_file, temp)) {
			_body += temp;
		}
		html_file.close();
		return true;
	} else if (!is_error_page) {
		_status.set_status_line(404, "Not Found");
		html(_server_data->common.errorPage.at(404), true);
	} else {
		send_error_page(404, "Not Found");
	}
	return false;
}

void ServerResponse::send_error_page(int code, std::string error_msg) {
	std::stringstream code_str;
	code_str << code;
	header("content-type", "text/html");
	if (!html(_server_data->common.errorPage.at(code), true)) {
		_status.set_status_line(code, error_msg);
		std::string err_msg = code_str.str() + " " + error_msg + " ";
		_body = "<!DOCTYPE html><html><head><title>" + err_msg +
				"</title></head>"
				"<body><h1>" +
				err_msg + "</h1></body></html>";
	}
}

std::string ServerResponse::generate_response() {
	bool found = false;
	for (size_t i = 0; i < _server_data->location.size(); ++i) {
		if (_req_data->uri_path.find(_server_data->location[i].path) == 0) {
			found = true;
			if ((_req_data->method == "GET" &&
				 _server_data->location[i].common.methods.getMethod) ||
				(_req_data->method == "POST" &&
				 _server_data->location[i].common.methods.postMethod) ||
				(_req_data->method == "DELETE" &&
				 _server_data->location[i].common.methods.deleteMethod)) {
				serve_static_page(_server_data->location[i],
								  _req_data->uri_path);
			} else {
				send_error_page(405, "Method Not Allowed");
				break;
			}
		}
	}
	if (!found) send_error_page(404, "Not Found");
	header("server", _server_data->server_name[0]); // looping through dif names?
	header("content-length", get_body_size());
	_response = WS_PROTOCOL + _status.status_line() + get_headers() + "\r\n" +
				get_body();
	return _response;
}

ServerResponse& ServerResponse::json(const std::string& data) {
	_body = data;
	return *this;
}

std::string ServerResponse::identify_mime() {
	if (_req_data->mime_type == "html") {
		_resp_content_type = "text/html";
	} else if (_req_data->mime_type == "css") {
		_resp_content_type = "text/css";
	} else if (_req_data->mime_type == "js") {
		_resp_content_type = "application/javascript";
	} else if (_req_data->mime_type == "json") {
		_resp_content_type = "application/json";
	} else if (_req_data->mime_type == "jpg" ||
			   _req_data->mime_type == "jpeg") {
		_resp_content_type = "image/jpeg";
	} else if (_req_data->mime_type == "png") {
		_resp_content_type = "image/png";
	} else if (_req_data->mime_type == "gif") {
		_resp_content_type = "image/gif";
	} else {
		_resp_content_type = "text/html";
	}
	return _resp_content_type;
}

const std::string ServerResponse::get_body_size() const {
	std::stringstream ss;
	ss << _body.size();
	return ss.str();
}

const std::string& ServerResponse::get_headers() const { return _headers; }

const std::string& ServerResponse::get_body() const { return _body; }