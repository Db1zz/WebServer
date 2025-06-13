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
#include "ServerResponse.hpp"

#define PAGE_INITIAL "Pages/inde.html"
#define PAGE_404 "Pages/404.html"
#define STYLESHEET "Pages/styles.css"

typedef struct s_request {
	std::string method;
	std::string uri_path;
	std::string user_agent;	 // dont need for now
	std::string host;
	std::string language;  // dont need for now
	std::string connection;
	std::string mime_type;	   // format that client can accept in response
	std::string content_type;  // format of data sent to the server

} t_request;
/*add function:
request_validator
-> check each variable of the request struct and generate (andor should return
empty string)*/

class ServerResponse {
   public:
	ServerResponse(const t_request& request);
	~ServerResponse();
	ServerResponse& operator<<(const std::string& data);
	ServerResponse& header(const std::string& key, const std::string& value);
	ServerResponse& status_line(const int code);
	ServerResponse& html(const std::string& path);
	ServerResponse& json(const std::string& data);
	std::string generate_response();
	std::string identify_mime();

	/*getters*/
	const std::string get_body_size() const;
	const std::string& get_status() const;
	const std::string& get_headers() const;
	const std::string& get_body() const;

   private:
	std::string _resp_content_type;
	std::string _status_line;
	std::string _body;
	std::string _headers;
	std::string _response;
	const t_request* _req_data;
	Status<> _status_msg;
	/*setters*/
	/*getters*/
};

#endif	// SERVER_SERVERRESPONSE_HPP
