#include "ServerResponse.hpp"

#include <sstream>

#include "Server.hpp"

ServerResponse::ServerResponse(const t_request& request) : _req_data(&request) {
	_status_line = " 200 OK\r\n";
}

ServerResponse::~ServerResponse() {}

ServerResponse& ServerResponse::operator<<(const std::string& data) {
	_body += data;
	return *this;
}

ServerResponse& ServerResponse::header(const std::string& key, const std::string& value) {
	_headers += key + ": " + value + "\r\n";
	return (*this);
}

ServerResponse& ServerResponse::status_line(const int code) {
	std::stringstream code_str;
	code_str << code;
	std::string msg = "";
	if (code == 200)
		msg = " OK\r\n";
	else if (code == 404)
		msg = " not found\r\n";
	else if (code == 500)
		msg = " internal server error\r\n";
	_status_line = " " + code_str.str() + msg;
	return (*this);
}

ServerResponse& ServerResponse::html(const std::string& path) {
	std::ifstream html_file;
	html_file.open(path.c_str());
	if (html_file.is_open()) {
		std::string temp;
		while (getline(html_file, temp)) {
			_body += temp;
		}
		html_file.close();
	} else
		status_line(404) << "file not found :c";

	// header("content-length", get_body_size());
	return *this;
}

std::string ServerResponse::generate_response() {
	status_line(200);
	header("content-type", _req_data->content_type);
	header("server", "comrades_webserv");
	//_resp_content_type = identify_mime();
	if (_req_data->content_type == "text/html")	 // change to _resp_content_type when done
		html(
			PAGE_INITIAL);	// change to uri_path and modify the function to pick the page
	else if (_req_data->mime_type == "application/json") {
		// add a separate function to pick the correct call for all types
	} else {
		status_line(406);
		(*this) << "Not Acceptable";
	}
	_response = WS_PROTOCOL + get_status() + get_headers() + "\r\n" + get_body();
	std::cout << GREEN400 "RESPONSE:\n" << _response << RESET << std::endl;
	return _response;
}

// std::string ServerResponse::identify_mime() {
// 	std::string mime_type = _req_data->mime_type;
// 	static const std::map<std::string, std::string> mime_types = {
// 		{"/", "text/html"},
// 		{".html", "text/html"},
// 		{".jpeg", "image/jpeg"},
// 		{".jpg", "image/jpeg"},
// 		{".ico", "image/x-icon"},
// 		{".png", "image/png"},
// 		{".bin", "application/octet-stream"},
// 		{".bmp", "image/bmp"},
// 		{".css", "text/css"},
// 		{".csv", "text/csv"},
// 		{".gif", "image/gif"},
// 		{".js", "text/javascript"},
// 		{".json", "application/json"},
// 		{".mp3", "audio/mpeg"},
// 		{".mp4", "video/mp4"},
// 		{".mpeg", "video/mpeg"},
// 		{".pdf", "application/pdf"},
// 		{".svg", "image/svg+xml"},
// 		{".txt", "text/plain"},
// 		{".wav", "audio/wav"},
// 		{".xhtml", "application/xhtml+xml"},
// 		{".tif", "image/tiff"}
// 	};

// 	std::map<std::string, std::string>::const_iterator it =
// mime_types.find(mime_type); 	if (it != mime_types.end()) 		return
// it->second; 	else { 		status_line(415); 		this << "unsupported media
// type";
// 	}
// }

const std::string ServerResponse::get_body_size() const {
	std::stringstream ss;
	ss << _body.size();
	return ss.str();
}

const std::string& ServerResponse::get_status() const { return _status_line; }

const std::string& ServerResponse::get_headers() const { return _headers; }

const std::string& ServerResponse::get_body() const { return _body; }
