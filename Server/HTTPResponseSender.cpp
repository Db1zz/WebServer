#include "HTTPResponseSender.hpp"

#include "ServerResponse.hpp"
#include "ServerSocketManager.hpp"

HTTPResponseSender::HTTPResponseSender(ClientSocket& client_socket, t_request* request,
																			 const t_config* server_config, ServerSocker& server_socket, ServerLogger* server_logger)
		: _client_socket(client_socket),
			_request(request),
			_server_config(server_config),
			_server_logger(server_logger),			
			_server_socket(server_socket) {
}

Status HTTPResponseSender::send(const Status& status) {
	ServerResponse resp(_request, *_server_config, status, &_server_socket.get_session_store());
	resp.generate_response();

	if (resp.status.code() == 100) {
		return Status();
	}
	if (resp.needs_streaming()) {
		std::string headers = resp.get_response();
		if (write(_client_socket.get_fd(), headers.c_str(), headers.size()) < 0)
			return Status("failed to send response headers to client");
		Status stream_status = Chunk::stream_file_chunked(
			resp.get_stream_file_path(), _client_socket.get_fd(), resp.get_stream_location());
		if (!stream_status.is_ok()) return stream_status;
	} else {
		std::string res = resp.get_response();
		if (write(_client_socket.get_fd(), res.c_str(), res.size()) < 0)
			return Status("failed to send response to client");
	}
	return Status::OK();
}