#ifndef SERVER_SERVER_EVENT_HPP
#define SERVER_SERVER_EVENT_HPP

#include "status.hpp"

#include <sys/epoll.h>

#include <map>
#include <vector>

#ifndef SERVER_EVENT_CLIENT_EVENTS
#define SERVER_EVENT_CLIENT_EVENTS (EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP)
#endif // SERVER_EVENT_CLIENT_EVENTS

#ifndef SERVER_EVENT_SERVER_EVENTS
#define SERVER_EVENT_SERVER_EVENTS (EPOLLIN | EPOLLOUT)
#endif // SERVER_EVENT_SERVER_EVENTS

#include "IIOContext.hpp"
#include "IIOHandler.hpp"

#include "DolbayobPTR.hpp"

class IEventContext;

class ServerEvent {
public:
    ServerEvent();
    ~ServerEvent();

    // takes ownership over event_context
    bool register_event(uint32_t events, int event_fd, DolbayobPTR<IEventContext> event_context);
    void unregister_event(int event_fd);

    Status wait_event(int timeout, int *nfds);
    void event_mod(uint32_t events, int event_fd);

    epoll_event *operator[](size_t index);
    size_t size();
    size_t capacity();

    DolbayobPTR<IEventContext> get_event_context(int event_fd);

private:
    void init();
    // Status resize_events_arr(size_t new_size);
    void copy_events_arr(size_t src_size, const epoll_event *src, epoll_event *dst);

    epoll_event *_events_arr;
    std::map<int, DolbayobPTR<IEventContext> > _events_contexts;
    std::vector<DolbayobPTR<IEventContext> > _closed_events_contexts;
    size_t _events_size;
    size_t _events_capacity;
    int _epoll_fd;
};

#endif  // SERVER_SERVER_EVENT_HPP