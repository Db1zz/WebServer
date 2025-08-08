#include "AccessLogger.hpp"

#include <unistd.h>

#include <iostream>
#include <stdexcept>

AccessLogger::AccessLogger(const std::string& access_logger_dir)
	: ALogger(access_logger_dir, "access"), _header_written(false) {
}

Status AccessLogger::write_logger_header() {
	Status status;
	std::string logger_header;

	logger_header = "#Version: " + get_logger_version() + "\n" +
					"#Fields: " + get_logger_fields() + "\n" +
					"#Date: " + get_current_date('-') + "\n";

	status = write_data_to_logfile(logger_header);
	if (!status) {
		Status("failed write_data_to_logfile(): " + status.msg());
	}
	return Status();
}

std::string AccessLogger::get_logger_version() {
	return std::string("1.0");
}
std::string AccessLogger::get_logger_fields() {
	return std::string("date time c-ip cs-method cs-uri s-port");
}

Status AccessLogger::log(const std::string& remote_host_ip, const std::string& method,
						 const std::string& requested_url, int server_port) {
	std::stringstream line;
	Status status;

	if (!_header_written) {
		write_logger_header();
		_header_written = true;
	}

	line << get_current_date('-') << " "
		<< get_current_time(':') << " "
		<< remote_host_ip << " "
		<< method << " "
		<< requested_url << " "
		<< server_port << "\n";

	status = write_data_to_logfile(line.str());
	return status;
}