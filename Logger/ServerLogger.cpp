#include "ServerLogger.hpp"

ServerLogger::ServerLogger(const std::string& logger_dir)
	: _access_logger(logger_dir), _error_logger(logger_dir) {
}

Status ServerLogger::init() {
	Status result;

	result = _access_logger.init();
	if (!result) {
		return result;
	}
	result = _error_logger.init();
	return result;
}

Status ServerLogger::log_error(const std::string& failed_component, const std::string& error) {
	return _error_logger.log(failed_component, error);
}
Status ServerLogger::log_access(const std::string& remote_host_ip, const std::string& method,
								const std::string& requested_url, int server_port) {
	return _access_logger.log(remote_host_ip, method, requested_url, server_port);
}