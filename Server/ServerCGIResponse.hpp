#ifndef SERVER_SERVER_CGI_RESPONSE_HPP_
#define SERVER_SERVER_CGI_RESPONSE_HPP_

#include <string>

#include "ServerConfig.hpp"
#include "ServerRequest.hpp"

class ServerCGIResponse {
   public:
	ServerCGIResponse(const t_request& request, const t_config& server_data);
	~ServerCGIResponse();
	const std::string& generate_response();

   private:
	static const std::string& get_env_var_name(size_t id);

	std::string _cgi_response;
	t_request _request;
	t_config _server_data;
};

#endif // SERVER_SERVER_CGI_RESPONSE_HPP_