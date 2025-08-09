#ifndef WEBSERVER_UTILITIES_FS_HPP
#define WEBSERVER_UTILITIES_FS_HPP

#include "status.hpp"

#include <string>
#include <dirent.h>
#include <typeinfo>
#include <fstream>

namespace fs {

Status is_dir_exist(const std::string &dirname);
Status is_file_exist(const std::string &filename);
Status open_file(std::fstream &file, const std::string &path, std::ios::openmode mode);
Status extract_filename_from_a_path(std::string &filename, const std::string &path);
Status normalize_path(std::string& out, const std::string &path_to_normalize);

}  // namespace fs

#endif  // WEBSERVER_UTILITIES_FS_HPP