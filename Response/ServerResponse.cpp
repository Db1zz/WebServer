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
	  _is_chunked(false),
	  _needs_streaming(false),
	  _stream_location(NULL) {
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
	status.set_status_line(status.code(), status.msg());
	if (_needs_streaming) {
		_response = WS_PROTOCOL + status.status_line() + get_headers() + "\r\n";
	} else {
		_response = WS_PROTOCOL + status.status_line() + get_headers() + "\r\n" + get_body();
	}
	//std::cout << "status: " << status.code() << std::endl;
	// std::cout << RED400 << "status: " << status.code() << RESET << std::endl;
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
	} else {
		status = Status::NotFound();
		_error_handler->send_error_page(404, "Not Found", _body, _headers);
	}
}

bool ServerResponse::serve_file(const std::string& path, bool is_error_page) {
	std::fstream file;
	fs::open_file(file, path, std::ios::in | std::ios::binary);

	if (!status.is_ok() || !file.is_open()) {
		if (file.is_open()) file.close();
		return _error_handler->handle_file_error(is_error_page, _body, _headers);
	}

	const t_location* location = _file_utils->find_best_location_match();
	_is_chunked = Chunk::is_chunked_response(_resolved_file_path, location);
	
	if (_is_chunked) {
		_needs_streaming = true;
		_stream_file_path = path;
		_stream_location = location;
		file.close();
		return true;
	}
	
	bool read_success = _file_utils->read_file_content(file, _body);
	if (!read_success) {
		return _error_handler->handle_file_error(is_error_page, _body, _headers);
	}
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

Status ServerResponse::generate_cgi_response(Status cgi_status, std::string& cgi_body) {
	//check if all body receive, if not, receive status continue? var ton check if cgi received?
	header("server", _server_data->server_name[0]);
	header("content-type", "text/html");
	_body = cgi_body;
	status = cgi_status;
	header("content-length", get_body_size());
	status.set_status_line(status.code(), status.msg());
	_response = WS_PROTOCOL + status.status_line() + get_headers() + "\r\n" + get_body();
	return status;
	
}

const std::string& ServerResponse::get_content_type() const {
	return _resp_content_type;
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
	while (!_req_data->content_data.empty()) {
		t_request_content &content_data = _req_data->content_data.front();
		std::string upload_dir = _resolved_file_path; //?call resolve_file_path again?
		FileUtils::ensureTrailingSlash(upload_dir);
	
		std::string file_path = upload_dir + content_data.filename;
	
		if (FileUtils::is_file_exists(file_path) && !content_data.is_file_created) {
			status = Status::Conflict();
			_json_handler->set_error_response("File already exists", _body, _headers);
			return ;
		}
		bool file_saved = _file_utils->save_uploaded_file(file_path, content_data);
		if (file_saved) {
			status = Status::OK();
			_json_handler->set_success_response("Upload successful", _body, _headers);
		} else if (content_data.is_finished) {
			status = Status::BadRequest();
			_json_handler->set_error_response("No file uploaded or failed to save file(s)", _body,
											  _headers);
		} else {
			content_data.is_file_created = true;
			status = Status::Continue();
		}
		if (content_data.is_finished) {
			_req_data->content_data.pop_front();
		}
		else if (!content_data.is_finished) {
			content_data.data.clear();
			break;	
		}
	}
}

void ServerResponse::handle_file_delete() {
	if (access(_resolved_file_path.c_str(), F_OK) != 0) {
		_json_handler->set_error_response("File not found", _body, _headers);
		status = Status::NotFound();
		return;
	}

	if (unlink(_resolved_file_path.c_str()) == 0) {
		_json_handler->set_success_response("File deleted successfully", _body, _headers);
		status = Status::OK();
	} else {
		_json_handler->set_error_response("Failed to delete file", _body, _headers);
		status = Status::InternalServerError();
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
		_error_handler->send_error_page(405, "Method Not Allowed", _body, _headers);
		status = Status::MethodNotAllowed();
	}
}

bool ServerResponse::needs_streaming() const {
	return _needs_streaming;
}

const std::string& ServerResponse::get_stream_file_path() const {
	return _stream_file_path;
}

const t_location* ServerResponse::get_stream_location() const {
	return _stream_location;
}
