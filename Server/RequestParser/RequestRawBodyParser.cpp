#include "RequestRawBodyParser.hpp"
#include <unistd.h> // REMOVE ME
#include <stdlib.h>

RequestRawBodyParser::RequestRawBodyParser(int content_length, RequestBodyStorageType type)
	: _content_length(content_length), _data_size(0), _type(type), _finished(false) {
	static uint64_t i;
	if (_type == InFile) {
		std::stringstream ss;
		ss << i;
		++i;
		_temp_file_name = std::string("./.tempfiles/") + ss.str(); // temporary solution in reality we need to create a hash value!!
		_fstream.open(_temp_file_name.c_str());
		if (!_fstream.is_open()) { // shit thing ... (just doing it for fun)
			std::cout << "ABOBA\n";
			exit(-1); // REMOVE ME(it's insane bro2)
		}
	}
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
}

bool RequestRawBodyParser::is_finished() const {
	return _finished;
}
