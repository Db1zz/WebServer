#ifndef SERVER_ERROR_RESPONSE_HPP
#define SERVER_ERROR_RESPONSE_HPP

#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "../Utilities/fs.hpp"
#include "../Utilities/status.hpp"
#include "ServerConfig.hpp"
#include "ServerRequest.hpp"

class ErrorResponse {
   public:
	ErrorResponse(t_request* req_data, Status& status, const t_config* server_data);
	~ErrorResponse();

	void send_error_page(int code, const std::string& error_msg, std::string& body,
						 std::string& headers);
	std::string get_error_page_path(int code) const;
	void generate_fallback_error_page(int code, const std::string& error_msg, std::string& body);
	std::string get_status_text(int code) const;
	bool handle_file_error(bool is_error_page, std::string& body, std::string& headers);

   private:
	t_request* _req_data;
	Status& _status;
	const t_config* _server_data;

	void add_header(const std::string& key, const std::string& value, std::string& headers);
	bool serve_error_file(const std::string& path, std::string& body);
};

#endif // SERVER_ERROR_RESPONSE_HPP