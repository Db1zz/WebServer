#ifndef WEBSERVER_UTILITIES_STATUS_HPP
#define WEBSERVER_UTILITIES_STATUS_HPP

#include <string>

class Status {
public:
	Status();
	Status(std::string error);
	Status(std::string error, int code);
	Status(std::string error, int code, bool ok);
	Status(const Status &to_copy);

	Status &operator=(const Status &to_copy);
	std::string operator+(const std::string &_msg);

	operator bool() const { return !_ok; }

	void set_ok(bool ok) { _ok = ok; }
	void set_msg(std::string msg) { _msg = msg; }
	void set_code(int code) { _code = code; }

	bool ok() { return _ok; }
	int code() const { return _code; }
	std::string &msg() { return _msg; }
	const std::string &msg() const { return _msg; }

protected:
	bool _ok;
	int _code;
	std::string _msg;
};

#endif  // WEBSERVER_UTILITIES_STATUS_HPP