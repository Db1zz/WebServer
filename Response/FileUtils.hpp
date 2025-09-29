#ifndef SERVER_FILE_UTILS_HPP
#define SERVER_FILE_UTILS_HPP

#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <string>

#include "ServerConfig.hpp"
#include "ServerRequest.hpp"

class FileUtils {
   public:
	FileUtils(t_request* req_data, const t_config* server_data);
	~FileUtils();

	static bool is_directory(const std::string& path);
	static void ensureTrailingSlash(std::string& path);
	static bool is_file_exists(const std::string& file_path);
	bool read_file_content(std::fstream& file, std::string& body);
	bool save_uploaded_file(const std::string& file_path, std::string &content_data);
	bool is_binary(const std::string& mime_type) const;
	std::string identify_mime_type() const;
	const t_location* find_best_location_match() const;
	void resolve_file_path(const t_location& location, std::string& resolved_path);
	std::string find_upload_directory() const;
	bool is_file_location(const t_location& location) const;

   private:
	t_request* _req_data;
	const t_config* _server_data;
};

#endif // SERVER_FILE_UTILS_HPP