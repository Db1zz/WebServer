#include "RequestRawBodyParser.hpp"
#include <fstream>

RequestRawBodyParser::RequestRawBodyParser(int content_length, RequestBodyStorageType type)
	: _data_size(0), _content_length(content_length), _type(type) {
	static uint64_t i;
	if (_type == InFile) {
		std::stringstream ss;
		ss << i;
		++i;
		_temp_file_name = ss.str();
		_fstream.open(_temp_file_name);
	}
}

RequestRawBodyParser::~RequestRawBodyParser() {
	_fstream.close();
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
	request.transfered_length += _data_size;
}
