#ifndef SERVER_CONNECTION_CONTEXT_HPP_
#define SERVER_CONNECTION_CONTEXT_HPP_

#include "ServerRequestParser.hpp"
#include <map>

class FileDescriptor;

class ConnectionState {
   public:
	enum State {
		IDLE,
		RECEIVING_REQUEST_HEADER_FROM_CLIENT,
		HANDLE_CGI_REQUEST,
		HANDLE_NORMAL_REQUEST,
		RESPONSE_SENT
	};

   private:
	ConnectionState();
};

struct ConnectionContext {
	t_request request;
	ServerRequestParser parser;
	ConnectionState::State state;
	std::string buffer;
	bool cgi_started;

	std::map<int, FileDescriptor*> descriptors;

	ConnectionContext::ConnectionContext()
		: request(), parser(&request), state(ConnectionState::IDLE), cgi_started(false) {}

	void ConnectionContext::reset() {
		request = t_request();
		parser = ServerRequestParser(&request);
		state = ConnectionState::IDLE;
		cgi_started = false;
		buffer.clear();
	}
};

#endif // SERVER_CONNECTION_CONTEXT_HPP_