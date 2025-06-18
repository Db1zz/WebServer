#include "AccessLogger.hpp"

#include <unistd.h>
#include <stdexcept>
#include <iostream>

AccessLogger::AccessLogger(std::string path)
{
	Status<> status = open_log_file();
	if (status) {
		throw std::runtime_error("AccessLogger Error: " + status);
	}
}

/*
	Goshan41k TODO

	impelemnt AccessLogger::write() function to log HTTP access
	https://en.wikipedia.org/wiki/Extended_Log_Format
	https://www.w3.org/Daemon/User/Config/Logging.html
*/
Status<> AccessLogger::write(/* connection data */) {
	// write connection data
}
