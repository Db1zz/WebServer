#ifndef SERVER_SERVERREQUEST_HPP
#define SERVER_SERVERREQUEST_HPP
#include <iostream>

typedef struct s_request {
	std::string method;
	std::string uri_path;
	std::string user_agent;	 // dont need for now
	std::string accept;
	std::string host;
	std::string language;  // dont need for now
	std::string connection;
	std::string mime_type;	   // format that client can accept in response

} t_request;

#endif