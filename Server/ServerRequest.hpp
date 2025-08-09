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
	std::string mime_type;	 // format that client can accept in response
	size_t contentLength;
	std::string fileContent; // this gets filled up in case of POST or DELETE method with the binary
							 // content of the file for further processing

} t_request;

#endif