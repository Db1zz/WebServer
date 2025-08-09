#include "ALogger.hpp"

#include <unistd.h>

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "fs.hpp"

ALogger::ALogger(const std::string& logger_dir, const std::string& logger_filename)
	: _logger_filename(logger_filename) {
	fs::normalize_path(_logger_dir, logger_dir);
}

ALogger::~ALogger() {
	if (_log_file_stream.is_open()) {
		_log_file_stream.close();
	}
}

Status ALogger::init() {
	std::string log_filename_with_date_and_time;

	log_filename_with_date_and_time =
		_logger_filename + "_" + get_current_date('-') + "_" + get_current_time(':') + ".txt";

	Status logfile_status = open_log_file(log_filename_with_date_and_time);
	return logfile_status;
}

Status ALogger::open_log_file(const std::string& log_filename_with_date_and_time) {
	return fs::open_file(_log_file_stream, _logger_dir + log_filename_with_date_and_time,
						 std::ios::out);
}

Status ALogger::write_data_to_logfile(const std::string& data) {
	try {
		_log_file_stream.write(data.c_str(), data.size());
	} catch (const std::exception& e) {
		return Status(e.what());
	}
	return Status();
}

std::string ALogger::get_current_date(char delimiter) {
	std::stringstream ss;
	std::time_t now = std::time(NULL);
	std::tm* local_time = std::localtime(&now);

	ss << (1900 + local_time->tm_year) << delimiter << std::setw(2) << std::setfill('0')
	   << (1 + local_time->tm_mon) << delimiter << std::setw(2) << std::setfill('0')
	   << local_time->tm_mday;

	return ss.str();
}

std::string ALogger::get_current_time(char delimiter) {
	std::stringstream ss;
	std::time_t now = std::time(NULL);
	std::tm* local_time = std::localtime(&now);

	ss << local_time->tm_hour << delimiter << std::setw(2) << std::setfill('0')
	   << local_time->tm_min << delimiter << std::setw(2) << std::setfill('0') << local_time->tm_sec
	   << std::setw(2) << std::setfill('0');

	return ss.str();
}
