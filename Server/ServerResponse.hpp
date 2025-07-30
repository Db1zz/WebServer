#ifndef SERVER_SERVER_RESPONSE_HPP
#define SERVER_SERVER_RESPONSE_HPP
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "../Utilities/colors.hpp"
#include "../Utilities/fs.hpp"
#include "../Utilities/status.hpp"
#include "Server.hpp"
#include "ServerConfig.hpp"
#include "ServerRequest.hpp"
#include "ServerResponse.hpp"

#define PAGE_INITIAL "Pages/index.html"
#define PAGE_404 "Pages/404.html"
#define STYLESHEET "Pages/styles.css"

class ServerResponse {
   public:
	ServerResponse(const t_request& request, const t_config& server_data);
	~ServerResponse();
	ServerResponse& operator<<(const std::string& data);
	ServerResponse& header(const std::string& key, const std::string& value);
	ServerResponse& serve_static_page(const t_location& loc,
									  const std::string& uri);
	ServerResponse& json(const std::string& data);
	ServerResponse& post_method(const std::string& data);
	ServerResponse& delete_method(const std::string& data);
	std::string generate_response();
	std::string identify_mime();
	bool serve_file(const std::string& path, bool is_error_page);
	bool is_binary();
	void send_error_page(int code, std::string error_msg);
	void serve_default_root();
	/*getters*/
	const std::string get_body_size() const;
	const std::string& get_headers() const;
	const std::string& get_body() const;

   private:
	std::string _resp_content_type;
	std::string _body;
	std::string _headers;
	std::string _response;
	const t_request* _req_data;
	const t_config* _server_data;
	Status _status;
	/*setters*/
	/*getters*/
};

#endif	// SERVER_SERVERRESPONSE_HPP
