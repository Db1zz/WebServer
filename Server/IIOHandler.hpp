#ifndef SERVER_I_IO_HANDLER_HPP_
#define SERVER_I_IO_HANDLER_HPP_

#include "status.hpp"
#include "ITimeoutTimer.hpp"
#include <sys/timerfd.h>

#define IO_HANDLER_TIMEOUT 5

class IIOHandler {
   public:
	virtual void handle(void* data) = 0;
	virtual bool is_closing() const = 0;
	virtual void set_is_closing() = 0;
	virtual void set_timeout_timer(ITimeoutTimer* timeout_timer) = 0;
	virtual ~IIOHandler() {};
};

#endif // SERVER_I_IO_HANDLER_HPP_