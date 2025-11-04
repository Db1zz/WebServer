#ifndef SERVER_I_IO_HANDLER_HPP_
#define SERVER_I_IO_HANDLER_HPP_

#include "status.hpp"

class IIOHandler {
   public:
	virtual Status handle(void* data) = 0;
	virtual bool is_closing() const = 0;
	virtual ~IIOHandler() {};
};

#endif // SERVER_I_IO_HANDLER_HPP_