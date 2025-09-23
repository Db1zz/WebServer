#include "JsonResponse.hpp"

JsonResponse::JsonResponse(t_request* req_data, Status& status)
	: _req_data(req_data), _status(status) {
}

JsonResponse::~JsonResponse() {
}

std::string JsonResponse::generate_directory(const std::string& directory_path) {
	DIR* dir = opendir(directory_path.c_str());
	if (dir == NULL) {
		return "[]";
	}

	std::string json_response = "[";
	struct dirent* entry;
	bool is_first = true;

	while ((entry = readdir(dir)) != NULL) {
		if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		if (!is_first) {
			json_response += ",";
		}

		json_response += "\"" + std::string(entry->d_name) + "\"";
		is_first = false;
	}

	closedir(dir);
	json_response += "]";

	return json_response;
}

std::string JsonResponse::create_json_response(const std::string& data, std::string& body,
											   std::string& headers) {
	if (data.empty()) {
		set_error_response("Internal Server Error - No file management location found", body,
						   headers);
		return "";
	}

	std::string json_response = generate_directory(data);
	body = json_response;
	add_header("content-type", "application/json", headers);
	add_header("access-control-allow-origin", "*", headers);

	return json_response;
}

void JsonResponse::set_success_response(const std::string& message, std::string& body,
										std::string& headers) {
	body = "{\"success\": true, \"message\": \"" + message + "\"}";
	add_header("content-type", "application/json", headers);
}

void JsonResponse::set_error_response( const std::string& message, std::string& body,
									  std::string& headers) {
	body = "{\"success\": false, \"message\": \"" + message + "\"}";
	add_header("content-type", "application/json", headers);
	add_header("access-control-allow-origin", "*", headers);
}

void JsonResponse::add_header(const std::string& key, const std::string& value,
							  std::string& headers) {
	headers += key + ": " + value + "\r\n";
}