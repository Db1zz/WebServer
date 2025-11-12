#ifndef SERVER_I_EVENT_CONTEXT_HPP_
#define SERVER_I_EVENT_CONTEXT_HPP_

class IIOHandler;
class IIOContext;
class FileDescriptor;
class ITimeoutTimer;

class IEventContext {
public:
	virtual ~IEventContext() {
	}

	virtual void take_data_ownership(IIOHandler* io_handler, IIOContext* io_context, FileDescriptor* fd, ITimeoutTimer* timer) = 0;
	virtual IIOHandler* get_io_handler() = 0;
	virtual IIOContext* get_io_context() = 0;
	virtual FileDescriptor* get_fd() = 0;
	virtual ITimeoutTimer* get_timer() = 0;
};

#endif // SERVER_I_EVENT_CONTEXT_HPP_