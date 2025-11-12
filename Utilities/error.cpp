#include "error.hpp"

Error::Error() : _error_code(Code::eNoError), _msg("No message") {
}

Error::Error(Code code) : _error_code(code), _msg("No message") {
}

Error::Error(Code code, const std::string& msg) : _error_code(code), _msg(msg) {
}

Error::operator bool() const {
	return _error_code == Code::eNoError;
}

bool Error::is_ok() const {
	return _error_code == Code::eNoError;
}

bool Error::is_fatal() const {
	return _error_code == Code::eFatalError;
}

Error Error::code() const {
	return _error_code;
}

bool Error::operator==(Code code) const {
	return _error_code == code;
}

bool Error::operator==(const Error& code) const {
	return _error_code == code._error_code;
}

const std::string& Error::msg() const {
	return _msg;
}

void Error::set_msg(const std::string& msg) {
	_msg = msg;
}
