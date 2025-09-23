#include "ErrorResponse.hpp"

ErrorResponse::ErrorResponse(t_request* req_data, Status& status, const t_config* server_data)
	: _req_data(req_data), _status(status), _server_data(server_data) {
}

ErrorResponse::~ErrorResponse() {
}

void ErrorResponse::send_error_page(int code, const std::string& error_msg, std::string& body,
									std::string& headers) {
	_status.set_status_line(code, error_msg);
	add_header("content-type", "text/html", headers);

	std::string error_page_path = get_error_page_path(code);

	if (!serve_error_file(error_page_path, body)) {
		generate_fallback_error_page(code, error_msg, body);
	}
}

std::string ErrorResponse::get_error_page_path(int code) const {
	std::map<int, std::string>::const_iterator it = _server_data->common.errorPage.find(code);

	if (it != _server_data->common.errorPage.end()) {
		return it->second;
	}

	return "";
}

void ErrorResponse::generate_fallback_error_page(int code, const std::string& error_msg,
												 std::string& body) {
	std::stringstream code_str;
	code_str << code;

	std::string error_message = code_str.str() + " " + error_msg + " ";

	body =
		"<!DOCTYPE html>"
		"<html>"
		"<head><title>" +
		error_message +
		"</title></head>"
		"<body><h1>" +
		error_message +
		"</h1></body>"
		"</html>";
}

std::string ErrorResponse::get_status_text(int code) const {
	switch (code) {
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 409:
			return "Conflict";
		case 500:
			return "Internal Server Error";
		case 400:
			return "Bad Request";
		default:
			return "Error";
	}
}

bool ErrorResponse::handle_file_error(bool is_error_page, std::string& body, std::string& headers) {
	if (!is_error_page) {
		_status.set_status_line(404, "Not Found");
		add_header("content-type", "text/html", headers);
		serve_error_file(_server_data->common.errorPage.at(404), body);
	}
	return false;
}

void ErrorResponse::add_header(const std::string& key, const std::string& value,
							   std::string& headers) {
	headers += key + ": " + value + "\r\n";
}

bool ErrorResponse::serve_error_file(const std::string& path, std::string& body) {
	if (path.empty()) {
		return false;
	}

	std::fstream file;
	Status file_status = fs::open_file(file, path, std::ios::in | std::ios::binary);

	if (!file_status.is_ok()) {
		return false;
	}

	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	body.resize(static_cast<size_t>(size));
	file.read(&body[0], size);
	file.close();

	return true;
}