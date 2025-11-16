#include "ServerEvent.hpp"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <stdexcept>

#include "ClientSocket.hpp"
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
}

bool ServerEvent::register_event(uint32_t events, int event_fd, DolbayobPTR<IEventContext> event_context) {
    if (_events_size == _events_capacity) {
        return false;
    }

    if (_events_contexts.find(event_fd) != _events_contexts.end()) {
        return true; // TODO
    }

    epoll_event new_event;

    new_event.data.ptr = event_context.get();
    new_event.events = events;

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, event_fd, &new_event) < 0) {
        throw SystemException(LOG_INFO(), "epoll_ctl() " + std::string(strerror(errno)));
    }
    _events_contexts.insert(std::make_pair(event_fd, event_context));
    ++_events_size;
    return true;
}

void ServerEvent::unregister_event(int event_fd) {
    std::map<int, DolbayobPTR<IEventContext> >::iterator it = _events_contexts.find(event_fd);
    if (it == _events_contexts.end()) {
        return;
    }

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, event_fd, NULL) < 0) {
        throw SystemException(LOG_INFO(), "epoll_ctl() " + std::string(strerror(errno)));
    }

    _closed_events_contexts.push_back(it->second);
    _events_contexts.erase(it);
    --_events_size;
    std::cout << "Connection with " << event_fd << " is closed\n";
}

Status ServerEvent::wait_event(int timeout, int* nfds) {
    _closed_events_contexts.clear();
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

epoll_event* ServerEvent::operator[](size_t index) {
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

DolbayobPTR<IEventContext> ServerEvent::get_event_context(int event_fd) {
	std::map<int, DolbayobPTR<IEventContext> >::iterator it = _events_contexts.find(event_fd);

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

void ServerEvent::copy_events_arr(size_t src_size, const epoll_event* src, epoll_event* dst) {
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
