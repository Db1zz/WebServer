#ifndef WEBSERVER_UTILITIES_STATUS_HPP
#define WEBSERVER_UTILITIES_STATUS_HPP

#include <string>

template <typename T = std::string>
class Status {
public:
	Status() : _ok(true) {}
	Status(T error) : _ok(false), _error(error) {}
	Status(T error, bool ok) : _ok(ok), _error(error) {}
	Status(const Status &to_copy) {
		*this = to_copy;
	}

	Status<T> &operator=(const Status<T> &to_copy) {
		if (this != &to_copy) {
			_ok = to_copy._ok;
			_error = to_copy._error;
		}
		return *this;
	}

	T operator+(const T &_error2) {
		return _error + _error2;
	}

	operator bool() const { return !_ok; }

	bool ok() const { return _ok; }
	T &get() { return _error; }
	const T &get() const { return _error; }

protected:
	bool _ok;
	T _error;
};

#endif  // WEBSERVER_UTILITIES_STATUS_HPP