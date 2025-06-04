#include "fs.hpp"

#include <stdexcept>
#include <iostream>
#include <dirent.h>
#include <cerrno>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

Status<> fs::is_dir_exist(const std::string &dirname) {
	struct stat s;
	if (stat(dirname.c_str(), &s) == 0 && S_ISDIR(s.st_mode)) {
		return Status<>();
	}
	return Status<>("Folder does not exist");
}

Status<> fs::is_file_exist(const std::string &filename) {
	if (access(filename.c_str(), F_OK) == 0) {
		return Status<>();
	}
	return Status<>("File does not exist");
}

Status<> fs::open_file(std::fstream &file, const std::string &path, std::ios::openmode mode) {
	if (path.empty()) {
		return Status<>("Path is empty");
	}
	try {
		file.open(path.c_str(), mode);
	} catch (const std::exception *e) {
		return Status<>(e->what());
	}
	if (!file.is_open()) {
		return Status<>("File does not exist");
	}
	return Status<>();
}

Status<> fs::extract_filename_from_a_path(std::string &filename, const std::string &path) {
	std::string::size_type start;

	if (path.empty()) {
		return Status<>("Path is empty");
	}
	start = path.find_last_of("/");
	if (start == std::string::npos) {
		filename.substr(0, path.size());
	} else {
		filename.substr(start, path.size() - start);
	}
	return Status<>();
}