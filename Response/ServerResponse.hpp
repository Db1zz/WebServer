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
#include "../Utilities/hash.hpp"
#include "Chunk.hpp"
#include "ClientSocket.hpp"
#include "ErrorResponse.hpp"
#include "FileUtils.hpp"
#include "JsonResponse.hpp"
#include "Server.hpp"
#include "../Server/SessionStore.hpp"
#include "ServerConfig.hpp"
#include "ServerRequest.hpp"
#include "ServerResponse.hpp"

#define PAGE_INITIAL "Pages/index.html"
#define PAGE_404 "Pages/404.html"
#define STYLESHEET "Pages/styles.css"

class JsonResponse;
class ErrorResponse;
class FileUtils;

class ServerResponse {
   public:
	ServerResponse(t_request* request, const t_config& server_data, const Status& status, SessionStore* session_store = NULL);
	~ServerResponse();

	ServerResponse& header(const std::string& key, const std::string& value);
	ServerResponse& handle_get_method(const t_location& loc);
	Status generate_response();
	Status generate_cgi_response();
	void serve_default_root();
	bool serve_file(const std::string& path, bool is_error_page);

	std::string get_body_size() const;
	const std::string& get_headers() const;
	const std::string& get_body() const;
	const std::string& get_response() const;
	const std::string& get_content_type() const;

	bool needs_streaming() const;
	const std::string& get_stream_file_path() const;
	const t_location* get_stream_location() const;

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

	bool _is_chunked;
	bool _needs_streaming;
	std::string _stream_file_path;
	const t_location* _stream_location;
	SessionStore* _session_store;

	void handle_directory(const t_location& location);
	void handle_file_upload();
	void handle_file_delete();
	void set_binary_headers();
	void choose_method(const t_location& location);

	void handle_auth_login();
	void handle_auth_session();
	void handle_auth_logout();
	std::string get_query_param(const std::string& key) const;
};

#endif