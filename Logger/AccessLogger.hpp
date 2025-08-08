#ifndef LOGGER_ACCESS_LOGGER_HPP
#define LOGGER_ACCESS_LOGGER_HPP

#include <string>

#include "ALogger.hpp"

#define ACCESS_LOGGER_EERRORN "AccessLogger"

class AccessLogger : public ALogger {
   public:
	explicit AccessLogger(const std::string& access_logger_dir);

	Status log(const std::string& remote_host_ip, const std::string& http_method,
			   const std::string& requested_url, int server_port);

   private:
	Status write_logger_header();
	std::string get_logger_version();
	std::string get_logger_fields();
	bool _header_written;
};
#endif // LOGGER_ACCESS_LOGGER_HPP