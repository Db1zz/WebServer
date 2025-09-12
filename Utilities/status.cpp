#include "status.hpp"

Status::Status() : _ok(true), _code(200), _msg("OK") {}

Status::Status(std::string error) : _ok(false), _code(200), _msg(error) {}

Status::Status(std::string error, int code)
	: _ok(false), _code(code), _msg(error) {}

Status::Status(std::string error, int code, bool ok)
	: _ok(ok), _code(code), _msg(error) {}

Status::Status(std::string func_name, const char *errmsg)
	: _ok(false), _code(200), _msg(func_name + errmsg) {}

Status::Status(const Status &to_copy) { *this = to_copy; }

Status &Status::operator=(const Status &to_copy) {
	if (this != &to_copy) {
		_ok = to_copy._ok;
		_code = to_copy._code;
		_msg = to_copy._msg;
	}
	return *this;
}

std::string Status::operator+(const std::string &_msg) { return _msg + _msg; }

bool Status::operator==(int code) {
	return _code == code;
}

void Status::set_status_line(int code, std::string msg) {
	std::stringstream code_str;
	_code = code;
	code_str << code;
	_msg = msg;
	_status_line = " " + code_str.str() + " " + msg + "\r\n";
}
