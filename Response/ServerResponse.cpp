#include "ServerResponse.hpp"

#include <vector>
#include "ErrorResponse.hpp"
#include "FileUtils.hpp"
#include "JsonResponse.hpp"

ServerResponse::ServerResponse(ClientSocket* client_socket, const t_config& server_data)
	: status(),
	  _server_data(&server_data),
	  _req_data(NULL),
	  _json_handler(NULL),
	  _error_handler(NULL),
	  _file_utils(NULL), 
	  _is_chunked(false) {
	if (client_socket != NULL) _req_data = &client_socket->get_connection_context()->request;
	_json_handler = new JsonResponse(_req_data, status);
	_error_handler = new ErrorResponse(_req_data, status, _server_data);
	_file_utils = new FileUtils(_req_data, _server_data);
}

ServerResponse::~ServerResponse() {
	delete _json_handler;
	delete _error_handler;
	delete _file_utils;
}

ServerResponse& ServerResponse::header(const std::string& key, const std::string& value) {
	_headers += key + ": " + value + "\r\n";
	return *this;
}

ServerResponse& ServerResponse::handle_get_method(const t_location& location) {
	if (FileUtils::is_directory(_resolved_file_path)) {
		handle_directory(location);
		return *this;
	}
	_resp_content_type = _file_utils->identify_mime_type();
	header("content-type", _resp_content_type);

	if (_file_utils->is_binary(_req_data->mime_type)) set_binary_headers();
	serve_file(_resolved_file_path, false);
	return *this;
}

Status ServerResponse::generate_response() {
	const t_location* best_match = _file_utils->find_best_location_match();

	if (best_match != NULL)
		choose_method(*best_match);
	else
		serve_default_root();

	header("server", _server_data->server_name[0]);
	if (_is_chunked)
		header("transfer-encoding", "chunked");
	else
		header("content-length", get_body_size());
	status.set_status_line();
	_response = WS_PROTOCOL + status.status_line() + get_headers() + "\r\n" + get_body();
	return status;
}

void ServerResponse::serve_default_root() {
	if (_req_data->uri_path == "/") {
		t_location default_location;
		default_location.common.root = _server_data->common.root;
		default_location.common.index = _server_data->common.index;
		default_location.path = "/";

		_file_utils->resolve_file_path(default_location, _resolved_file_path);
		handle_get_method(default_location);
	} else
		_error_handler->send_error_page(404, "Not Found", _body, _headers);
}

bool ServerResponse::serve_file(const std::string& path, bool is_error_page) {
	std::fstream file;
	fs::open_file(file, path, std::ios::in | std::ios::binary);

	if (!status.is_ok()) return _error_handler->handle_file_error(is_error_page, _body, _headers);

	const t_location* location = _file_utils->find_best_location_match();
	_is_chunked = is_chunked_response(location);
	
	if (_is_chunked)
		return serve_file_chunked(file, location);
	return _file_utils->read_file_content(file, _body);
}

bool ServerResponse::serve_file_chunked(std::fstream& file, const t_location* location) {
	if (!file.is_open()) {
		return false;
	}
	size_t chunk_size = (location && location->chunked_size > 0) ? location->chunked_size : 8192;
	std::vector<char> buffer(chunk_size);
	std::string chunked_content;

	while (file.good() && !file.eof()) {
		file.read(buffer.data(), chunk_size);
		std::streamsize bytes_read = file.gcount();
		
		if (bytes_read > 0) {
			std::string chunk_data(buffer.data(), bytes_read);
			chunked_content += Chunk::encode(chunk_data);
		}
	}
	chunked_content += Chunk::generate_final_chunk();
	_body = chunked_content;
	
	file.close();
	return true;
}

std::string ServerResponse::get_body_size() const {
	std::stringstream ss;
	ss << _body.size();
	return ss.str();
}

const std::string& ServerResponse::get_headers() const {
	return _headers;
}

const std::string& ServerResponse::get_body() const {
	return _body;
}

const std::string& ServerResponse::get_response() const {
	return _response;
}

void ServerResponse::handle_directory(const t_location& location) {
	if (location.common.auto_index &&
		(_req_data->mime_type == ".json" || _req_data->accept == "*/*")) {
		_json_handler->create_json_response(_resolved_file_path, _body, _headers);
		return;
	}
	if (!_resolved_file_path.empty() &&
		_resolved_file_path[_resolved_file_path.size() - 1] != '/') {
		_resolved_file_path += "/";
	}
	_resolved_file_path += location.common.index.empty() ? "index.html" : location.common.index[0];

	_req_data->mime_type = ".html";
	_resp_content_type = _file_utils->identify_mime_type();
	header("content-type", _resp_content_type);
	serve_file(_resolved_file_path, false);
}

void ServerResponse::set_binary_headers() {
	header("content-disposition", "inline");
	header("cache-control", "public, max-age=3600");
}

void ServerResponse::handle_file_upload() {
	std::string upload_dir = _resolved_file_path;
	FileUtils::ensureTrailingSlash(upload_dir);

	std::string file_path = upload_dir + _req_data->filename;

	if (FileUtils::is_file_exists(file_path) && !_req_data->is_file_created) {
		status = Status::Conflict();
		_json_handler->set_error_response("File already exists", _body, _headers);
		return;
	}

	bool file_saved = _file_utils->save_uploaded_file(file_path);
	if (file_saved) {
		status = Status::OK();
		_json_handler->set_success_response("Upload successful", _body, _headers);
	} else if (_req_data->is_request_ready()) {
		status = Status::BadRequest();
		_json_handler->set_error_response("No file uploaded or failed to save file(s)", _body,
										  _headers);
	} else {
		_req_data->is_file_created = true;
		status = Status::Continue();
	}
}

void ServerResponse::handle_file_delete() {
	if (access(_resolved_file_path.c_str(), F_OK) != 0) {
		status = Status::NotFound();
		_json_handler->set_error_response("File not found", _body, _headers);
		return;
	}

	if (unlink(_resolved_file_path.c_str()) == 0) {
		status = Status::OK();
		_json_handler->set_success_response("File deleted successfully", _body, _headers);
	} else {
		status = Status::InternalServerError();
		_json_handler->set_error_response("Failed to delete file", _body, _headers);
	}
}

void ServerResponse::choose_method(const t_location& location) {
	_file_utils->resolve_file_path(location, _resolved_file_path);

	if (_req_data->method == "DELETE" && location.common.methods.deleteMethod) {
		handle_file_delete();
	} else if (_req_data->method == "GET" && location.common.methods.getMethod) {
		handle_get_method(location);
	} else if (_req_data->method == "POST" && location.common.methods.postMethod) {
		handle_file_upload();
	} else {
		status = Status::MethodNotAllowed();
		_error_handler->send_error_page(405, "Method Not Allowed", _body, _headers);
	}
}

bool ServerResponse::is_chunked_response(const t_location* location) const {
	if (location && location->chunked_transfer_encoding) {
		size_t file_size = get_file_size(_resolved_file_path);
		return file_size >= location->chunked_threshold;
	}
	return false;
}

size_t ServerResponse::get_file_size(const std::string& file_path) const {
	std::ifstream file(file_path.c_str(), std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		return 0;
	}
	return static_cast<size_t>(file.tellg());
}
