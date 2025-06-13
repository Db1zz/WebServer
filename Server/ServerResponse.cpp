#include "ServerResponse.hpp"

#include <sstream>

#include "Server.hpp"

ServerResponse::ServerResponse(const t_request& request) : _req_data(&request) {
	_status_line = "200";
	// maybe set a default msg in status directly?
}

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

ServerResponse& ServerResponse::status_line(const int code) {
	std::stringstream code_str;
	code_str << code;
	std::string msg = _status_msg.get();
	if (msg == "") {
		msg = " OK";
	};
	_status_line = " " + code_str.str() + " " + msg + "\r\n";
	return (*this);
}

/*TODO: check ngnix behavior and codes and implement the same*/

ServerResponse& ServerResponse::html(const std::string& path) {
	std::fstream html_file;
	_status_msg = fs::open_file(html_file, path, std::ios::in);
	if (_status_msg.ok()) {
		std::string temp;
		while (getline(html_file, temp)) {
			_body += temp;
		}
		html_file.close();
	} else
		status_line(404);  // create error class inheriting serverResponse and
						   // send its body when error
	return *this;
}

std::string ServerResponse::generate_response() {
	status_line(200);
	_resp_content_type = identify_mime();
	header("content-type", _resp_content_type);
	header("server", "comrades_webserv");
	if (_resp_content_type == "text/html")
		html(PAGE_INITIAL);
	else if (_resp_content_type == "application/json") {
		json("json response");
	} else {
		status_line(406);
		(*this) << "not acceptable\r\n";
		/*TODO: figure out how to still serve a page with incorrect requests
		2. check how different data types are sent(when not html)*/
	}
	header("content-length", get_body_size());
	_response =
		WS_PROTOCOL + get_status() + get_headers() + "\r\n" + get_body();
	std::cout << GREEN400 "RESPONSE:\n" << _response << RESET << std::endl;
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
		_resp_content_type = "application/octet-stream";
	}
	return _resp_content_type;
}

const std::string ServerResponse::get_body_size() const {
	std::stringstream ss;
	ss << _body.size();
	return ss.str();
}

const std::string& ServerResponse::get_status() const { return _status_line; }

const std::string& ServerResponse::get_headers() const { return _headers; }

const std::string& ServerResponse::get_body() const { return _body; }