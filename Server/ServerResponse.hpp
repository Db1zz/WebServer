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
#include "ClientSocket.hpp"
#include "Server.hpp"
#include "ServerConfig.hpp"
#include "ServerRequest.hpp"
#include "ServerResponse.hpp"

#define PAGE_INITIAL "Pages/index.html"
#define PAGE_404 "Pages/404.html"
#define STYLESHEET "Pages/styles.css"

class ClientSocket;
class ServerResponse {
   public:
	ServerResponse(ClientSocket* client_socket, const t_config& server_data);
	~ServerResponse();
	ServerResponse& operator<<(const std::string& data);
	ServerResponse& header(const std::string& key, const std::string& value);
	ServerResponse& serve_static_page(const t_location& loc);
	ServerResponse& json(const std::string& data);
	ServerResponse& post_method(const t_location& loc);
	ServerResponse& delete_method(const t_location& loc);
	ServerResponse& handle_api_files();
	std::string generate_response();
	std::string identify_mime();
	bool serve_file(const std::string& path, bool is_error_page);
	bool is_binary();
	void send_error_page(int code, std::string error_msg);
	void serve_default_root();
	void resolve_file_path(const t_location& loc);
	/*getters*/
	const std::string get_body_size() const;
	const std::string& get_headers() const;
	const std::string& get_body() const;
	Status _status;

   private:
	std::string _resp_content_type;
	std::string _body;
	std::string _headers;
	std::string _response;
	std::string _resolved_file_path;
	t_request* _req_data;
	const t_config* _server_data;
	/*setters*/
	/*getters*/
};

#endif // SERVER_SERVERRESPONSE_HPP