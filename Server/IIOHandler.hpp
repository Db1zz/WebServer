#ifndef SERVER_I_IO_HANDLER_HPP_
#define SERVER_I_IO_HANDLER_HPP_

#include "status.hpp"

#define IO_HANDLER_TIMEOUT 5

class IIOHandler {
   public:
	virtual Status handle(void* data) = 0;
	virtual bool is_closing() const = 0;
	virtual ~IIOHandler() {};
};

#endif // SERVER_I_IO_HANDLER_HPP_