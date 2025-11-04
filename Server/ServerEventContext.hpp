#ifndef SERVER_SERVER_EVENT_CONTEXT_HPP_
#define SERVER_SERVER_EVENT_CONTEXT_HPP_

#include <unistd.h>

#include "IEventContext.hpp"

class IIOHandler;
class IIOContext;
class FileDescriptor;

class ServerEventContext : public IEventContext {
public:
	ServerEventContext() : _io_handler(NULL), _io_context(NULL), _fd(NULL) {}
	~ServerEventContext() {
		delete _io_handler;
		delete _io_context;
		delete _fd;
	}

	void take_data_ownership(IIOHandler* io_handler, IIOContext* io_context, FileDescriptor* fd) {
		delete _io_handler;
		delete _io_context;
		delete _fd;

		_io_handler = io_handler;
		_io_context = io_context;
		_fd = fd;
	}
	
	IIOHandler* get_io_handler() { return _io_handler; }
	IIOContext* get_io_context() { return _io_context; }
	FileDescriptor* get_fd() { return _fd; }

private:
	IIOHandler* _io_handler;
	IIOContext* _io_context;
    FileDescriptor* _fd;
};

#endif // SERVER_SERVER_EVENT_CONTEXT_HPP_