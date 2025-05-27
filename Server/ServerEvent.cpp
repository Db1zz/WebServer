#include "ServerEvent.hpp"

#include <unistd.h>
#include <stdexcept>
#include <string.h>

ServerEvent::ServerEvent()
    : _events_arr(NULL), _events_size(0), _events_capacity(5)
{
    init();
}

ServerEvent::ServerEvent(uint32_t events, int event_fd) {
    ServerEvent();
    add_event(events, event_fd);
}

ServerEvent::~ServerEvent() {
    close(_epoll_fd);

    if (_events_arr) {
        delete[] _events_arr;
    }
}

void ServerEvent::add_event(uint32_t events, int event_fd) {
    epoll_event new_event;

    new_event.data.fd = event_fd;
    new_event.events = events;

    if (_events_size == _events_capacity) {
        resize_events_arr(_events_capacity * 2);
    }

    epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, event_fd, &new_event);
    ++_events_size;
}

/*
    The same rules are applied to timeout as for epoll_wait()

    read about it: man epoll_wait
*/
int ServerEvent::wait_event(int timeout) {
    int nfds = epoll_wait(_epoll_fd, _events_arr, _events_capacity, _epoll_fd);
    if (nfds < 0) {
        std::runtime_error("epoll_wait() failed: " + std::string(strerror(errno)));
    }
    return nfds;
}

epoll_event *ServerEvent::operator[](size_t index) {
    if (index > _events_size) {
        std::runtime_error("Error in ServerEvent::operator[]: index > _events_size");
    }
    return &(_events_arr[index]);
}

void ServerEvent::init() {
    _epoll_fd = epoll_create(1);
    _events_arr = new epoll_event[_events_capacity];
}

void ServerEvent::resize_events_arr(size_t new_size) {
    epoll_event *new_events_arr = new epoll_event[new_size];
    copy_events_arr(_events_capacity, _events_arr, new_events_arr);
    delete[] _events_arr;
    _events_arr = new_events_arr;
    _events_capacity = new_size;
}

void ServerEvent::copy_events_arr(size_t src_size, const epoll_event *src, epoll_event *dst) {
    for (size_t i = 0; i < src_size; ++i) {
        dst[i] = src[i];
    }
}