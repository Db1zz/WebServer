#ifndef SERVER_JSON_RESPONSE_HPP
#define SERVER_JSON_RESPONSE_HPP

#include <dirent.h>

#include <cstring>
#include <string>

#include "../Utilities/status.hpp"
#include "ServerRequest.hpp"

class JsonResponse {
   public:
	JsonResponse(t_request* req_data, Status& status);
	~JsonResponse();

	std::string generate_directory(const std::string& directory_path);
	std::string create_json_response(const std::string& data, std::string& body,
									 std::string& headers);
	void set_success_response(int code, const std::string& message, std::string& body,
							  std::string& headers);
	void set_error_response(int code, const std::string& message, std::string& body,
							std::string& headers);

   private:
	t_request* _req_data;
	Status& _status;
	std::string get_status_text(int code) const;
	void add_header(const std::string& key, const std::string& value, std::string& headers);
};

#endif // SERVER_JSON_RESPONSE_HPP