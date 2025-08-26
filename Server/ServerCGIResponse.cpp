#include "ServerCGIResponse.hpp"

ServerCGIResponse::ServerCGIResponse(const t_request& request, const t_config& server_data)
	: _request(request), _server_data(server_data) {
}

ServerCGIResponse::~ServerCGIResponse() {
}

// std::string cgi_query_string; -> set env variables -> execute script?
// These environment variables include details such as the request method (GET or POST), query
// parameters, HTTP headers, and more. The CGI script can access these environment variables to
// gather information about the request.

// More information can be found under this link:
// https://www.ibm.com/docs/en/netcoolomnibus/8.1.0?topic=scripts-environment-variables-in-cgi-script

const std::string& ServerCGIResponse::get_env_var_name(size_t id) {
	static std::vector<std::string> env_vars = {"CONTENT_LENGTH",
												"CONTENT_TYPE",
												"GATEWAY_INTERFACE",
												"HTTP_ACCEPT",
												"HTTP_ACCEPT_CHARSET",
												"HTTP_ACCEPT_ENCODING",
												"HTTP_ACCEPT_LANGUAGE",
												"HTTP_FORWARDED",
												"HTTP_HOST",
												"HTTP_PROXY_AUTHORIZATION",
												"HTTP_USER_AGENT",
												"PATH_INFO",
												"PATH_TRANSLATED",
												"QUERY_STRING",
												"REMOTE_ADDR",
												"REMOTE_HOST",
												"REMOTE_USER",
												"REQUEST_METHOD",
												"SCRIPT_NAME",
												"SERVER_NAME",
												"SERVER_PORT",
												"SERVER_PROTOCOL",
												"SERVER_SOFTWARE"};
	if (id > env_vars.size()) {
		return std::string();
	}
	return env_vars[id];
}

const std::string& ServerCGIResponse::generate_response() {
	
}