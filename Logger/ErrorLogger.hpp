#ifndef WEBSERVER_ERROR_LOGGER_HPP_
#define WEBSERVER_ERROR_LOGGER_HPP_

#include <string>

#include "ALogger.hpp"

class ErrorLogger : public ALogger {
   public:
	explicit ErrorLogger(const std::string& error_logger_dir);

	Status log(const std::string& failed_component, const std::string& error);
};

#endif // WEBSERVER_ERROR_LOGGER_HPP_