#include "ALogger.hpp"
#include "fs.hpp"

#include <fstream>
#include <unistd.h>
#include <stdexcept>
#include <iostream>

ALogger::~ALogger() {
	if (_log_file.is_open()) {
		_log_file.close();
	}
}

Status ALogger::open_log_file() {
	std::fstream file;
	return fs::open_file(file, _path, std::ios::out);
}

const std::string &ALogger::get_last_log() {
	// TODO
	// return last log line...
}
