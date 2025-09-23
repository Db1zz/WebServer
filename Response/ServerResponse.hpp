#ifndef SERVER_SERVER_RESPONSE_HPP
#define SERVER_SERVER_RESPONSE_HPP
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "../Utilities/colors.hpp"
#include "../Utilities/fs.hpp"
#include "../Utilities/status.hpp"
#include "Chunk.hpp"
#include "ClientSocket.hpp"
#include "ErrorResponse.hpp"
#include "FileUtils.hpp"
#include "JsonResponse.hpp"
#include "Server.hpp"
#include "ServerConfig.hpp"
#include "ServerRequest.hpp"
#include "ServerResponse.hpp"

#define PAGE_INITIAL "Pages/index.html"
#define PAGE_404 "Pages/404.html"
#define STYLESHEET "Pages/styles.css"

class ClientSocket;
class JsonResponse;
class ErrorResponse;
class FileUtils;

class ServerResponse {
   public:
	ServerResponse(ClientSocket* client_socket, const t_config& server_data);
	~ServerResponse();

	ServerResponse& header(const std::string& key, const std::string& value);
	ServerResponse& serve_static_page(const t_location& loc);
	Status generate_response();
	void serve_default_root();
	bool serve_file(const std::string& path, bool is_error_page);

	std::string get_body_size() const;
	const std::string& get_headers() const;
	const std::string& get_body() const;
	const std::string& get_response() const;

	// bool is_chunked_response(const t_location* location) const;
	// size_t get_file_size(const std::string& file_path) const;

	Status status;

   private:
	const t_config* _server_data;
	t_request* _req_data;

	JsonResponse* _json_handler;
	ErrorResponse* _error_handler;
	FileUtils* _file_utils;

	std::string _resp_content_type;
	std::string _body;
	std::string _headers;
	std::string _response;
	std::string _resolved_file_path;

	void handle_directory(const t_location& location);
	void handle_file_upload();
	void handle_file_delete();
	void set_binary_headers();
	void choose_method(const t_location& location);
};

#endif