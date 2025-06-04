#ifndef LOGGER_ALOGGER_HPP
#define LOGGER_ALOGGER_HPP

#include "status.hpp"

#include <fstream>
#include <string>

class ALogger {
public:
	virtual ~ALogger();

	virtual Status<> write() = 0;
	virtual const std::string &get_last_log();

protected:
	Status<> ALogger::open_log_file();

	std::string _file_name;
	std::string _path;
	std::string _last_log_line;
	std::fstream _log_file;
};

#endif  // LOGGER_ALOGGER_HPP
