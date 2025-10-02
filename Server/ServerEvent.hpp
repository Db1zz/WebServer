#ifndef SERVER_SERVER_EVENT_HPP
#define SERVER_SERVER_EVENT_HPP

#include "status.hpp"

#include <sys/epoll.h>

#ifndef SERVER_EVENT_CLIENT_EVENTS
#define SERVER_EVENT_CLIENT_EVENTS (EPOLLIN | EPOLLRDHUP | EPOLLOUT | EPOLLERR | EPOLLHUP)
#endif // SERVER_EVENT_CLIENT_EVENTS

#ifndef SERVER_EVENT_SERVER_EVENTS
#define SERVER_EVENT_SERVER_EVENTS (EPOLLIN | EPOLLOUT)
#endif // SERVER_EVENT_SERVER_EVENTS


class FileDescriptor;

class ServerEvent {
public:
    ServerEvent();
    ServerEvent(uint32_t events, int event_fd);
    ~ServerEvent();

	Status add_event(uint32_t events, FileDescriptor* fd);
	Status add_event(uint32_t events, int event_fd);
    Status remove_event(int event_fd);
    Status wait_event(int timeout, int *nfds);
    Status event_mod(uint32_t events, int event_fd);

    epoll_event *operator[](size_t index);

private:
    Status init();
    Status resize_events_arr(size_t new_size);
    void copy_events_arr(size_t src_size, const epoll_event *src, epoll_event *dst);

    epoll_event *_events_arr;
    size_t _events_size;
    size_t _events_capacity;
    int _epoll_fd;
};

#endif  // SERVER_SERVER_EVENT_HPP