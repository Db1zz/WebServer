#ifndef SERVER_SERVER_EXCEPTION_HPP_
#define SERVER_SERVER_EXCEPTION_HPP_

#include <exception>
#include <string>

#include "ExceptionsUtils.hpp"

class ServerException : public std::exception {
public:
	ServerException(std::string exception_location, std::string error) {
		_msg = "ServerException throwed with a error " + error + "\n" + "in " + exception_location;
	}
	virtual ~ServerException() throw() {}

	virtual const char* what() const throw() {
		return _msg.c_str();
	};

private:
	std::string _msg;
};


#endif // SERVER_SERVER_EXCEPTION_HPP_