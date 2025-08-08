#include "ErrorLogger.hpp"

ErrorLogger::ErrorLogger(const std::string& error_logger_dir)
	: ALogger(error_logger_dir, "error") {
}

Status ErrorLogger::log(const std::string& failed_component, const std::string& error) {
	std::stringstream line;
	Status status;

	line << get_current_date('-') << " "
		<< get_current_time(':') << " "
	   	<< "[" << failed_component << "] Error: "
		<< error << "\n";

	status = write_data_to_logfile(line.str());
	return status;
}