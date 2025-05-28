#ifndef SERVER_SERVER_RESPONSE_HPP
#define SERVER_SERVER_RESPONSE_HPP
#include <iostream>
#include <map>

#include "Utilities/colors.hpp"

class ServerResponse {
   public:
	ServerResponse();
	~ServerResponse();
	ServerResponse& operator<<(const std::string& data);

   private:
	std::string _status;
	std::string _body;
	std::map<std::string, std::string> _headers;

	/*setters*/
	/*getters*/
};

#endif	// SERVER_SERVERRESPONSE_HPP
