#ifndef WEBSERVER_UTILITIES_STATUS_HPP
#define WEBSERVER_UTILITIES_STATUS_HPP

#include <string>
#include <sstream>

class Status {
   public:
	Status();
	Status(std::string error);
	Status(std::string error, int code);
	Status(std::string error, int code, bool ok);
	Status(std::string func_name, const char *errmsg);
	Status(const Status &to_copy);

	Status &operator=(const Status &to_copy);
	std::string operator+(const std::string &_msg);

	operator bool() const { return _ok; }

	void set_ok(bool ok) { _ok = ok; }
	void set_msg(std::string msg) { _msg = msg; }
	void set_code(int code) { _code = code; }
	void set_status_line(int code, std::string msg);

	bool ok() { return _ok; }
	int code() const { return _code; }
	std::string &msg() { return _msg; }
	const std::string &msg() const { return _msg; }
	const std::string &status_line() const { return _status_line; }

   protected:
	bool _ok;
	int _code;
	std::string _msg;
	std::string _status_line;
};

#endif	// WEBSERVER_UTILITIES_STATUS_HPP