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

ServerResponse& ServerResponse::serve_static_page() {
	// loop through locations -> find matching uri path
	// check if method is allowed for this location(write a separate function
	// that will be passed in every function with different types of a request,
	// set status to 405 Method Not Allowed or proceed)
	// function to build the path
	// build a function serve_static_page
	// build a function serve dynamic_page
	return *this;
};

ServerResponse& ServerResponse::html(const std::string& path,
									 bool is_error_page) {
	std::fstream html_file;
	_status = fs::open_file(html_file, path, std::ios::in);
	if (_status.ok()) {
		std::string temp;
		while (getline(html_file, temp)) {
			_body += temp;
		}
		html_file.close();
	} else if (is_error_page) {
		_status.set_status_line(404, "Not Found");
		html(_server_data->common.errorPage.at(404), false);
	} else {
		/*modify this part later, add default page to serve if config files fail*/
		_status.set_status_line(404, "Not Found");
		_body = "<h1>404 Not Found</h1>"; //need this for debugging purposes for now
	}
	return *this;
}

std::string ServerResponse::generate_response() {
	_status.set_status_line(200, "OK");
	_resp_content_type = identify_mime();
	header("content-type", _resp_content_type);
	header("server", "comrades_webserv");
	if (_resp_content_type == "text/html")
		html(_server_data->location[0].path, false);
	else if (_resp_content_type == "application/json") {
		json("json response");
	} else {
		_status.set_status_line(406, "Not Acceptable");
		html(_server_data->common.errorPage.at(406), true);
	}
	header("content-length", get_body_size());
	_response = WS_PROTOCOL + _status.status_line() + get_headers() + "\r\n" +
				get_body() + "\r\n";
	;
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