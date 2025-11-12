#include "status.hpp"

#include <sstream>

Status::Status()
	: _error_type(::OK),
	  _error_code(static_cast<int>(::OK)),
	  _error_msg("OK"),
	  _status_line("200 OK"),
	  _is_error(false) {
}

Status::Status(const std::string& error_msg)
	: _error_type(CustomError),
	  _error_code(static_cast<int>(CustomError)),
	  _error_msg(error_msg),
	  _status_line(error_msg),
	  _is_error(true) {
}

Status::Status(ErrorCode error_type, int error_code, const std::string& error_msg, bool is_error)
	: _error_type(error_type),
	  _error_code(error_code),
	  _error_msg(error_msg),
	  _is_error(is_error) {

	set_status_line(error_code, error_msg);
	if (!is_error) {
		_is_error = default_is_error_for_code(error_code);
	}
}

Status::Status(const Status& to_copy)
	: _error_type(to_copy._error_type),
	  _error_code(to_copy._error_code),
	  _error_msg(to_copy._error_msg),
	  _status_line(to_copy._status_line),
	  _is_error(to_copy._is_error) {
}

Status& Status::operator=(const Status& to_copy) {
	if (this == &to_copy) {
		return *this;
	}
	_error_type = to_copy._error_type;
	_error_code = to_copy._error_code;
	_error_msg = to_copy._error_msg;
	_status_line = to_copy._status_line;
	_is_error = to_copy._is_error;
	return *this;
}

bool Status::default_is_error_for_code(int code) {
	return (code >= 400 && code < 600);
}

bool Status::operator==(int code) const {
	return _error_code == code;
}
bool Status::operator==(ErrorCode error_type) const {
	return _error_type == error_type;
}
bool Status::operator==(const Status& status) const {
	return _error_type == status._error_type && _error_code == status._error_code;
}

Status::operator bool() const {
	return !_is_error;
}

bool Status::is_ok() const {
	return !_is_error;
}

const std::string& Status::msg() const {
	return _error_msg;
}
ErrorCode Status::error() const {
	return _error_type;
}
int Status::code() const {
	return _error_code;
}

void Status::set_status_line(int error_code, std::string error_msg) {
	std::stringstream code_str;
	code_str << error_code;

	_status_line = " " + code_str.str() + " " + error_msg + "\r\n";
}

void Status::set_status_line_cgi(std::string msg) {
	_status_line = " " + msg + "\r\n";
}

const std::string Status::status_line() {
	return _status_line;
}
