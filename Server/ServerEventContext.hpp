#ifndef SERVER_SERVER_EVENT_CONTEXT_HPP_
#define SERVER_SERVER_EVENT_CONTEXT_HPP_

#include <unistd.h>

#include "IEventContext.hpp"

class IIOHandler;
class IIOContext;
class FileDescriptor;
class ITimeoutTimer;

class ServerEventContext : public IEventContext {
public:
	ServerEventContext() : _io_handler(NULL), _io_context(NULL), _fd(NULL), _timer(NULL) {}
	~ServerEventContext() {
		delete _io_handler;
		delete _io_context;
		delete _timer;
	}

	void take_data_ownership(IIOHandler* io_handler, IIOContext* io_context, FileDescriptor* fd, ITimeoutTimer* timer) {
		delete _io_handler;
		delete _io_context;
		delete _fd;
		delete _timer;

		_io_handler = io_handler;
		_io_context = io_context;
		_fd = fd;
		_timer = timer;
	}
	
	IIOHandler* get_io_handler() { return _io_handler; }
	IIOContext* get_io_context() { return _io_context; }
	FileDescriptor* get_fd() { return _fd; }
	ITimeoutTimer* get_timer() { return _timer; }

private:
	IIOHandler* _io_handler;
	IIOContext* _io_context;
    FileDescriptor* _fd;
	ITimeoutTimer* _timer;
};

#endif // SERVER_SERVER_EVENT_CONTEXT_HPP_