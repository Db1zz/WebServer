#include "ServerResponse.hpp"

ServerResponse::ServerResponse() {
	_status = "200 OK\r\n";
}

ServerResponse::~ServerResponse() {}

ServerResponse& ServerResponse::operator<<(const std::string& data) {
		_body += data;
		return *this;
}
