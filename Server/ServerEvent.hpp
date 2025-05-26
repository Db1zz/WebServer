#ifndef SERVER_SERVER_EVENT_HPP
#define SERVER_SERVER_EVENT_HPP

#include <sys/epoll.h>

class ServerEvent {
public:
    ServerEvent();
    ServerEvent(uint32_t events, int event_fd);
    ~ServerEvent();

	void add_event(uint32_t events, int event_fd);

    // TODO
    // void remove_event();
    // void modify_event();

private:
    void init();
    void resize_events_arr(size_t new_size);
    void copy_events_arr(size_t src_size, const epoll_event *src, epoll_event *dst);

    epoll_event *_events_arr;
    size_t _events_size;
    size_t _events_capacity;

    int _epoll_fd;
};

#endif  // SERVER_SERVER_EVENT_HPP