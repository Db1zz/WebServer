#ifndef SERVER_SERVERREQUEST_HPP
#define SERVER_SERVERREQUEST_HPP

#include <iostream>
#include <string>
#include <list>

typedef struct s_request_content {
	std::string filename;
	std::string name;
	std::string content_type;
	std::string data;

	bool is_finished;
} t_request_content;

typedef struct s_request {
	std::string method;
	std::string protocol_version;
	std::string uri_path;
	std::string uri_path_params;
	std::string user_agent;
	std::string accept;
	std::string host;
	std::string language; // dont need for now
	std::string connection;
	std::string mime_type; // format that client can accept in response
	std::string cgi_query_string;
	size_t content_length;
	std::string content_type;
	std::string boundary;

	size_t transfered_length; // length of the whole body that was transfered
	std::string filename;

	std::string body_chunk;
	std::string cache;
	std::list<t_request_content> content_data;
	bool is_file_created;

	bool is_request_ready() const { return transfered_length >= content_length; }

	s_request() : content_length(0), transfered_length(0), is_file_created(false) {}
} t_request;

#endif