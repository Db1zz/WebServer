#ifndef SERVER_IO_SERVER_CONTEXTP_HPP_
#define SERVER_IO_SERVER_CONTEXTP_HPP_

#include "IIOContext.hpp"

#include <map>
#include "IEventContext.hpp"
#include "DolbayobPTR.hpp"

class IOServerContext : public IIOContext {
	public:
		IOServerContext() {}
		~IOServerContext() {
		}

	void reset() {
	}
};

#endif // SERVER_IO_SERVER_CONTEXTP_HPP_