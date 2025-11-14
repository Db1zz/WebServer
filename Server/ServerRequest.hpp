#ifndef SERVER_SERVERREQUEST_HPP
#define SERVER_SERVERREQUEST_HPP

#include <stdint.h>

#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

class RequestType {
public:
	enum {
		STANDARD_REQUEST,
		CGI_REQUEST
	};
};

typedef struct s_request_content {
	std::string filename; // only filename extracted from the bottom part of the header 
	std::string name; //dont use
	std::string content_type; // image/png
	std::string data; //content

	bool is_file_created; //i modify
	bool is_finished; // all data is sent to response, request finished
	s_request_content() : is_file_created(false), is_finished(false) {}
} t_request_content;

typedef struct s_media_type {
	std::string type;
	std::string subtype;
	std::map<std::string, std::string> parameters;

	const std::string* find_parameter(const std::string& parameter_name) {
		std::map<std::string, std::string>::const_iterator it;
		it = parameters.find(parameter_name);
		if (it == parameters.end()) {
			return NULL;
		}

		return &it->second;
	}
} t_media_type;

typedef struct s_accept_item {

	t_media_type media_type;
	std::vector<std::pair<std::string, std::string> > accept_ext;
	double q;
	size_t order;

	s_accept_item() : q(1.0), order(UINT32_MAX) {}
} t_accept_item;

// typedef struct s_uri_path {
// 	std::string filename;
// 	std::string mime_type;
// 	std::string path;
// 	std::string path_parameter;
// 	std::vector<std::string> queries;
// 	std::string fragment;
// } t_uri_path;

typedef struct s_request {
	std::string session_id; //parse from client if exists, if not set to ""
	std::string method;
	std::string protocol_version;
	std::string user_agent;
	std::string accept;
	std::string host;
	std::string language; // dont need for now
	std::string connection;
	uint64_t content_length;
	t_media_type content_type;
	// std::string boundary;

	uint64_t transfered_length; // length of the whole request that was transfered

	std::string body_chunk;
	std::string cache;
	// t_uri_path uri_path_data;

	std::string filename; //filename extracted from filename, dint use 
	std::string mime_type;
	std::string uri_path;
	std::string path_parameter; //dont use for now
	std::vector<std::string> path_queries; //cgi
	std::vector<std::string> transfer_encoding;
	std::string status_string;

	std::list<t_request_content> content_data;

	bool is_file_created; // TODO remove me
	bool is_chunked_request;
	RequestType requesst_type;
	std::string cgi_bin;
	bool is_cgi;
	
	bool is_streaming;
	size_t stream_offset;
	std::string stream_file_path;
	std::string stream_pending_data;
	size_t stream_pending_sent;
	bool stream_headers_sent;

	bool is_request_ready() const { return transfered_length >= content_length; }

	s_request()
		: connection("keep-alive"),
		  content_length(0),
		  transfered_length(0),
		  is_chunked_request(false),
		  is_cgi(false),
		  is_streaming(false),
		  stream_offset(0),
		  stream_file_path(""),
		  stream_pending_data(""),
		  stream_pending_sent(0),
		  stream_headers_sent(false) {}
} t_request;

#endif