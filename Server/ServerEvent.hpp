#ifndef SERVER_SERVER_EVENT_HPP
#define SERVER_SERVER_EVENT_HPP

#include "status.hpp"

#include <sys/epoll.h>

class ServerEvent {
public:
    ServerEvent();
    ServerEvent(uint32_t events, int event_fd);
    ~ServerEvent();

	Status add_event(uint32_t events, int event_fd);
    Status remove_event(uint32_t events, int event_fd);
    Status wait_event(int timeout, int *nfds);

    epoll_event *operator[](size_t index);
    // void modify_event();

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