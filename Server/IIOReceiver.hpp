#ifndef SERVER_I_IO_RECEIVER_HPP_
#define SERVER_I_IO_RECEIVER_HPP_

#include "status.hpp"

class IIOReceiver {
   public:
	virtual ~IIOReceiver() {};
	virtual Status receive() = 0;
};

#endif // SERVER_I_IO_RECEIVER_HPP_