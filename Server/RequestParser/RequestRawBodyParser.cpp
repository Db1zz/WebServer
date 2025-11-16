#include "RequestRawBodyParser.hpp"
#include <unistd.h> // REMOVE ME
#include <stdlib.h>

RequestRawBodyParser::RequestRawBodyParser(int content_length, RequestBodyStorageType type)
	: _content_length(content_length), _data_size(0), _type(type), _finished(false) {
}

RequestRawBodyParser::~RequestRawBodyParser() {
	if (_fstream.is_open()) {
		_fstream.close();
	}
}

Status RequestRawBodyParser::feed(const std::string& content, size_t start_pos) {
	Status status = Status::Incomplete();
	int content_size = content.size() - start_pos;

	if (_data_size + content_size > _content_length) {
		content_size = _content_length - _data_size;
		status = Status::OK();
	}

	if (_type == InBuffer) {
		_data.append(content.substr(start_pos, content_size));
	} else if (_type == InFile) {
		_fstream.write(content.c_str() + start_pos, content_size);
	}
	_data_size += content_size;

	return status;
}

void RequestRawBodyParser::apply(t_request& request) {
	request.transfered_length += _data_size;
	if (_type == InBuffer) {
		request.body_chunk = _data;
	}
}

bool RequestRawBodyParser::is_finished() const {
	return _finished;
}
