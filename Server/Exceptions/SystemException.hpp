#ifndef SERVER_EXCEPTION_SYSTEM_EXCEPTION_HPP_
#define SERVER_EXCEPTION_SYSTEM_EXCEPTION_HPP_

#include <exception>
#include <string>

#include "ExceptionsUtils.hpp"

class SystemException : public std::exception {
public:
	SystemException(std::string exception_location, std::string error) {
		_msg = "SystemException throwed with a error " + error + "\n" + "in " + exception_location;
	}
	virtual ~SystemException() throw() {}

	virtual const char* what() const throw() {
		return _msg.c_str();
	};

private:
	std::string _msg;
};

#endif // SERVER_EXCEPTION_SYSTEM_EXCEPTION_HPP_