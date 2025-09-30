#include "RequestRawBodyParser.hpp"

RequestRawBodyParser::RequestRawBodyParser(int content_length, RequestBodyStorageType type)
	: _data_size(0), _content_length(content_length), _type(type) {
}

Status RequestRawBodyParser::feed(const std::string& content, size_t start_pos) {
	Status status = Status::Incomplete();
	int content_size = content.size() - start_pos;

	if (_data_size + content_size > _content_length) {
		content_size = _content_length - _data_size;
		status = Status::OK();
	}

	_data = content.substr(start_pos, content_size);
	_data_size += content_size;

	return status;
}

void RequestRawBodyParser::apply(t_request& request) {
	request.body_chunk.append(_data);
	request.transfered_length += _data_size;
}
