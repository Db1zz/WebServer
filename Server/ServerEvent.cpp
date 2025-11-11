#include "ServerEvent.hpp"

#include <unistd.h>
#include <stdexcept>
#include <string.h>
#include <errno.h>

#include "Socket.hpp"
#include "IIOContext.hpp"
#include "IEventContext.hpp"
#include "ClientSocket.hpp"
#include "Exceptions/SystemException.hpp"

ServerEvent::ServerEvent()
    : _events_arr(NULL), _events_size(0), _events_capacity(2000)
{
    init();
}

ServerEvent::~ServerEvent() {
    close(_epoll_fd);

    if (_events_arr) {
        delete[] _events_arr;
    }
    destroy_remaining_events();
}

bool ServerEvent::register_event(uint32_t events, int event_fd, IEventContext* event_context) {
    if (_events_size == _events_capacity) {
        return false;
    }

    epoll_event new_event;

    new_event.data.ptr = event_context;
    new_event.events = events;

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, event_fd, &new_event) < 0) {
        return Status(strerror(errno));
    }
    _events_contexts.insert(std::make_pair(event_fd, event_context));
    ++_events_size;
    return true;
}

void ServerEvent::unregister_event(int event_fd) {
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, event_fd, NULL) < 0) {
        throw SystemException(LOG_INFO(), "epoll_ctl() " + std::string(strerror(errno)));
    }

    std::map<int, IEventContext*>::iterator it = _events_contexts.find(event_fd);
    if (it != _events_contexts.end()) {
        _events_contexts.erase(it);
    }
    --_events_size;
}

/*
    The same rules are applied to timeout as for epoll_wait()

    read about it: man epoll_wait
*/
Status ServerEvent::wait_event(int timeout, int *nfds) {
    *nfds = epoll_wait(_epoll_fd, _events_arr, _events_capacity, timeout);
    if (*nfds < 0) {
        if (errno == EINTR) {
			return Status::Interrupted();
		}
		return Status(UnknownError, errno, strerror(errno));
	}
    return Status::OK();
}

#include <iostream>

epoll_event *ServerEvent::operator[](size_t index) {
	if (index > _events_capacity) {
		throw std::runtime_error("Error in ServerEvent::operator[]: index > _events_size");
	}
	return &(_events_arr[index]);
}

size_t ServerEvent::size() {
    return _events_size;
}

size_t ServerEvent::capacity() {
	return _events_capacity;
}

IEventContext* ServerEvent::get_event_context(int event_fd) {
    std::map<int, IEventContext*>::iterator it = _events_contexts.find(event_fd);

    if (it == _events_contexts.end()) {
        return NULL;
    }

    return it->second;
}

void ServerEvent::init() {
    _epoll_fd = epoll_create(1);
    if (_epoll_fd < 0) {
        throw SystemException(LOG_INFO(), "epoll_create() " + std::string(strerror(errno)));
    }

    _events_arr = new epoll_event[_events_capacity];
}

// Status ServerEvent::resize_events_arr(size_t new_size) {
//     epoll_event *new_events_arr;

//     try {
//         new_events_arr = new epoll_event[new_size];
//         copy_events_arr(_events_capacity, _events_arr, new_events_arr);
//         delete[] _events_arr;
//         _events_arr = new_events_arr;
//         _events_capacity = new_size;
//     } catch (const std::exception &e) {
//         return Status(e.what());
//     }

//     return Status::OK();
// }

void ServerEvent::copy_events_arr(size_t src_size, const epoll_event *src, epoll_event *dst) {
    for (size_t i = 0; i < src_size; ++i) {
        dst[i] = src[i];
    }
}

void ServerEvent::event_mod(uint32_t events, int event_fd) {
    int error;
    epoll_event event;

    event.data.fd = event_fd;
    event.events = events;

    error = epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, event_fd, &event);
    if (error < 0) {
        throw SystemException(LOG_INFO(), "epoll_ctl() " + std::string(strerror(errno)));
    }
}

const std::map<int, IEventContext*>& ServerEvent::get_events_contexts() {
    return _events_contexts;
}

void ServerEvent::destroy_remaining_events() {
    if (_events_contexts.size() == 0) {
        return;
    }
        
    std::map<int, IEventContext*>::iterator it = _events_contexts.begin();
    while (it != _events_contexts.end()) {
        std::map<int, IEventContext*>::iterator to_delete = it;
        ++it;
        delete to_delete->second;
        _events_contexts.erase(to_delete);
    }
}