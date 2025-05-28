#ifndef SERVER_SERVER_RESPONSE_HPP
#define SERVER_SERVER_RESPONSE_HPP
#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <sstream>
#include "../Utilities/colors.hpp"
#include "ServerResponse.hpp"

class ServerResponse {
   public:
	ServerResponse();
	~ServerResponse();
	ServerResponse& operator<<(const std::string& data);
	ServerResponse& header(const std::string& key, const std::string& value);
	ServerResponse& status_line(const int code);
	ServerResponse& html(const std::string &path);

	/*getters*/
	const std::string get_body_size() const;
	const std::string &get_status() const;
	const std::string &get_headers() const;
	const std::string &get_body() const;

   private :
	std::string _status_line;
	std::string _body;
	std::string _headers;

	/*setters*/
	/*getters*/
};

#endif	// SERVER_SERVERRESPONSE_HPP
