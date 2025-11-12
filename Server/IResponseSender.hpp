#ifndef SERVER_I_RESPONSE_SENDER_HPP_
#define SERVER_I_RESPONSE_SENDER_HPP_

#include "status.hpp"

class IResponseSender {
public:
	virtual ~IResponseSender() {}
	virtual Status send(const Status& status) = 0;
};

#endif // SERVER_I_RESPONSE_SENDER_HPP_