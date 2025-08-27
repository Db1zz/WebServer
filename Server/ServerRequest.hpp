#ifndef SERVER_SERVERREQUEST_HPP
#define SERVER_SERVERREQUEST_HPP
#include <iostream>

typedef struct s_request {
	std::string method;
	std::string uri_path;
	std::string user_agent;
	std::string accept;
	std::string host;
	std::string language; // dont need for now
	std::string connection;
	std::string mime_type;	   // format that client can accept in response
	std::string cgi_query_string;
	int content_length;
	std::map <std::string, std::string> files; // key is file name
	std::string content_type;
	std::string bound;

} t_request;

#endif