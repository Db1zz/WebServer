#ifndef SERVER_IO_EPOLL_CONTEXT_HPP_
#define SERVER_IO_EPOLL_CONTEXT_HPP_

#include "IIOContext.hpp"

class IOEpollContext : public IIOContext {
public:
	int events;
};

#endif // SERVER_IO_EPOLL_CONTEXT_HPP_