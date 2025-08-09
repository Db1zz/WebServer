#ifndef WEBSERVER_LOGGER_ALOGGER_HPP_
#define WEBSERVER_LOGGER_ALOGGER_HPP_

#include <fstream>
#include <string>

#include "status.hpp"

class ALogger {
   public:
	virtual ~ALogger();

	virtual Status init();

   protected:
	ALogger(const std::string& logger_dir, const std::string& logger_filename);

	Status open_log_file(const std::string& log_filename_with_date_and_time);
	Status write_data_to_logfile(const std::string& data);
	virtual std::string get_current_date(char delimiter);
	virtual std::string get_current_time(char delimiter);

	std::string _logger_filename;
	std::string _logger_dir;
	std::string _last_log_line;
	std::fstream _log_file_stream;
};
#endif // WEBSERVER_LOGGER_ALOGGER_HPP_
