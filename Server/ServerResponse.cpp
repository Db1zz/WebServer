#include "ServerResponse.hpp"
#include <sstream>


ServerResponse::ServerResponse() { _status_line = "200 OK\r\n"; }

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
	_status_line = code_str.str() + " OK\r\n";
	/* TODO: add up different status_texts depending on a code */
	return (*this);
}

ServerResponse& ServerResponse::html(const std::string& path) {
	header("content-type", "text/html");
	header("server", "comrades_webserv");
	/*add date? mandatory?*/
	std::ifstream html_file;
	html_file.open(path.c_str());
		if(html_file.is_open()){
			std::string temp;
			while (getline(html_file, temp)) {
	   	 		_body += temp;
			}
			html_file.close();
		} else
			status_line(404) << "file not found :c";
	
	header("content-length", get_body_size());
		return *this;
}

const std::string ServerResponse::get_body_size() const {
	std::stringstream ss;
	ss << _body.size();
	return ss.str();
}

const std::string& ServerResponse::get_status() const {
	return _status_line;
}

const std::string& ServerResponse::get_headers() const {
	return _headers;
}

const std::string& ServerResponse::get_body() const {
	return _body;
}
