#ifndef LOGGER_SERVER_LOGGER_HPP_
#define LOGGER_SERVER_LOGGER_HPP_

#include <string>

#include "AccessLogger.hpp"
#include "ErrorLogger.hpp"
#include "status.hpp"

class ServerLogger {
   public:
	ServerLogger(const std::string& logger_dir);

	Status init();

	Status log_error(const std::string& failed_component, const std::string& error);
	Status log_access(const std::string& remote_host_ip, const std::string& method,
					  const std::string& requested_url, int server_port);

   private:
	AccessLogger _access_logger;
	ErrorLogger _error_logger;
};

#endif // LOGGER_SERVER_LOGGER_HPP_