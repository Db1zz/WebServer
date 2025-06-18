#ifndef LOGGER_ACCESS_LOGGER_HPP
#define LOGGER_ACCESS_LOGGER_HPP

#include "ALogger.hpp"

#include <string>

#define ACCESS_LOGGER_EERRORN "AccessLogger"

class AccessLogger : public ALogger {
public:
	AccessLogger(std::string path);

	Status<> write();
};

#endif  // LOGGER_ACCESS_LOGGER_HPP