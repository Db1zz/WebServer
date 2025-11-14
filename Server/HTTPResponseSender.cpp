#include "HTTPResponseSender.hpp"

#include "ServerSocket.hpp"
#include "ServerResponse.hpp"
#include "../Response/Chunk.hpp"
#include <unistd.h>

HTTPResponseSender::HTTPResponseSender(ClientSocket& client_socket, t_request* request,
																			 const t_config* server_config, ServerSocket& server_socket, ServerLogger* server_logger)
		: _client_socket(client_socket),
			_request(request),
			_server_config(server_config),
			_server_logger(server_logger),			
			_server_socket(&server_socket) {
}

Status HTTPResponseSender::send(const Status& status) {
	ServerResponse resp(_request, *_server_config, status, &_server_socket->get_session_store());
	Status response_status = resp.generate_response();

	if (resp.status.code() == 100) {
		return Status();
	}
	if (resp.needs_streaming()) {
		std::string headers = resp.get_response();
		if (!_request->stream_headers_sent) {
			if (write(_client_socket.get_fd(), headers.c_str(), headers.size()) < 0)
				return Status("failed to send response headers to client");
			_request->stream_headers_sent = true;
		}
		if (!_request->is_streaming) {
			_request->is_streaming = true;
			_request->stream_offset = 0;
			_request->stream_file_path = resp.get_stream_file_path();
			_request->stream_pending_data.clear();
			_request->stream_pending_sent = 0;
		}

		bool finished = false;
		Status stream_status = Chunk::stream_file_chunked(
			_request->stream_file_path,
			_client_socket.get_fd(),
			resp.get_stream_location(),
			_request->stream_offset,
			finished,
			&_request->stream_pending_data,
			&_request->stream_pending_sent);
		if (!stream_status.is_ok()) return stream_status;

		if (finished && _request->stream_pending_data.empty()) {
			_request->is_streaming = false;
			_request->stream_offset = 0;
			_request->stream_file_path.clear();
			_request->stream_headers_sent = false;
		}
	} else {
		std::string res = resp.get_response();
		if (write(_client_socket.get_fd(), res.c_str(), res.size()) < 0)
			return Status("failed to send response to client");
	}
	return response_status;
}