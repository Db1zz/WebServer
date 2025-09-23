#include "status.hpp"

Status::Status() {
	*this = Status::OK();
}

Status::Status(std::string error_msg)
	: _error_type(::CustomError), _error_code((int) _error_type), _error_msg(error_msg) {
}

Status::Status(ErrorCode error_type, int error_code, std::string error_msg)
	: _error_type(error_type), _error_code(error_code), _error_msg(error_msg) {
}

Status::Status(const Status& to_copy) {
	*this = to_copy;
}

Status& Status::operator=(const Status& to_copy) {
	if (this != &to_copy) {
		_error_type = to_copy._error_type;
		_error_code = to_copy._error_code;
		_error_msg = to_copy._error_msg;
	}
	return *this;
}

bool Status::operator==(int error_code) {
	return _error_code == error_code;
}

bool Status::operator==(ErrorCode error_type) {
	return _error_type == error_type;
}

bool Status::operator==(Status status) {
	return _error_type == status._error_type;
}

Status::operator bool() const {
	return _error_type == ::OK;
}

bool Status::is_ok() {
	return _error_type == ::OK;
}

const std::string& Status::msg() {
	return _error_msg;
}

ErrorCode Status::error() {
	return _error_type;
}

void Status::set_status_line() {
	std::stringstream code_str;
	code_str << _error_code;
	_status_line = " " + code_str.str() + " " + _error_msg + "\r\n";
}

const std::string Status::status_line() {
	return _status_line;
}