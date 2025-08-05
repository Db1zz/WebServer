#include "fs.hpp"

#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <stdexcept>

namespace fs {
Status is_dir_exist(const std::string& dirname) {
	struct stat s;
	if (stat(dirname.c_str(), &s) == 0 && S_ISDIR(s.st_mode)) {
		return Status();
	}
	return Status("Folder does not exist");
}

Status is_file_exist(const std::string& filename) {
	if (access(filename.c_str(), F_OK) == 0) {
		return Status();
	}
	return Status("File does not exist");
}

Status open_file(std::fstream& file, const std::string& path, std::ios::openmode mode) {
	if (path.empty()) {
		return Status("Path is empty");
	}
	try {
		file.open(path.c_str(), mode);
	} catch (const std::exception* e) {
		return Status(e->what());
	}
	if (!file.is_open()) {
		return Status("File does not exist");
	}
	return Status();
}

Status extract_filename_from_a_path(std::string& filename, const std::string& path) {
	std::string::size_type start;

	if (path.empty()) {
		return Status("Path is empty");
	}
	start = path.find_last_of("/");
	if (start == std::string::npos) {
		filename.substr(0, path.size());
	} else {
		filename.substr(start, path.size() - start);
	}
	return Status();
}

Status normalize_path(std::string& out, const std::string& path_to_normalize) {
	const size_t path_size = path_to_normalize.size();

	if (path_size == 0) {
		return Status("path is empty");
	}

	if (path_to_normalize[path_size - 1] != '/') {
		out.reserve(path_size + 1);
	} else {
		out.reserve(path_size);
	}

	size_t curr_pos = 0;
	size_t last_pos = 0;
	size_t new_pos = 0;
	while (true) {
		curr_pos = path_to_normalize.find("//", last_pos);
		if (curr_pos == std::string::npos) {
			out.append(path_to_normalize, last_pos, path_size - last_pos);
			break;
		}
		new_pos = path_to_normalize.find_first_not_of("//", curr_pos);
		out.append(path_to_normalize, last_pos, curr_pos - last_pos + 1);
		last_pos = new_pos;
	}
	if (path_to_normalize[path_size - 1] != '/') {
		out.append("/");
	}
	return Status();
}
} // namespace fs
