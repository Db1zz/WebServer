#ifndef SERVER_CONNECTION_CONTEXT_HPP_
#define SERVER_CONNECTION_CONTEXT_HPP_

#include "RequestParser/ServerRequestParser.hpp"
#include "CGIResponseParser.hpp"
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
	CGIResponseParser* opt_cgi_parser;

	ConnectionState::State state;
	std::string buffer;
	bool cgi_started;

	std::map<int, FileDescriptor*> descriptors;

	ConnectionContext()
		: request(), parser(&request), opt_cgi_parser(NULL), state(ConnectionState::IDLE), cgi_started(false) {}

	void reset() {
		request = t_request();
		parser = ServerRequestParser(&request);
		if (opt_cgi_parser) {
			delete opt_cgi_parser;
			opt_cgi_parser = NULL;
		}
		state = ConnectionState::IDLE;
		cgi_started = false;
		buffer.clear();
	}
};

#endif // SERVER_CONNECTION_CONTEXT_HPP_