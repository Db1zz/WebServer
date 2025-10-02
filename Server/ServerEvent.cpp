#include "ServerEvent.hpp"

#include <unistd.h>
#include <stdexcept>
#include <string.h>
#include <errno.h>

#include "Socket.hpp"

ServerEvent::ServerEvent()
    : _events_arr(NULL), _events_size(100024), _events_capacity(5)
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

Status ServerEvent::add_event(uint32_t events, FileDescriptor* fd) {
	epoll_event new_event;

	new_event.data.ptr = fd;
	new_event.events = events;

	if (_events_size == _events_capacity) {
		resize_events_arr(_events_capacity * 2);
	}

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd->get_fd(), &new_event) < 0) {
		return Status(strerror(errno));
	}
	++_events_size;
    return Status::OK();
}

Status ServerEvent::add_event(uint32_t events, int event_fd) {
    epoll_event new_event;

    new_event.data.fd = event_fd;
    new_event.events = events;

    if (_events_size == _events_capacity) {
        resize_events_arr(_events_capacity * 2);
    }

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, event_fd, &new_event) < 0) {
        return Status(strerror(errno));
    }
    ++_events_size;
    return Status::OK();
}

Status ServerEvent::remove_event(int event_fd) {
    /*
        EPOLL_CTL_DEL
            Remove (deregister) the target file descriptor fd from the
            interest list.  The event argument is ignored and can be
            NULL (but see BUGS below).
    */
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, event_fd, NULL) < 0) {
        return Status(strerror(errno));
    }
    --_events_size;
    return Status::OK();
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

epoll_event *ServerEvent::operator[](size_t index) {
    if (index > _events_size) {
        throw std::runtime_error("Error in ServerEvent::operator[]: index > _events_size");
    }
    return &(_events_arr[index]);
}

Status ServerEvent::init() {
    _epoll_fd = epoll_create(1);
    if (_epoll_fd < 0) {
        return Status(strerror(errno));
    }

    try {
        _events_arr = new epoll_event[_events_capacity];
    } catch (const std::exception &e) {
        return Status(e.what());
    }

    return Status::OK();
}

Status ServerEvent::resize_events_arr(size_t new_size) {
    epoll_event *new_events_arr;

    try {
        new_events_arr = new epoll_event[new_size];
        copy_events_arr(_events_capacity, _events_arr, new_events_arr);
        delete[] _events_arr;
        _events_arr = new_events_arr;
        _events_capacity = new_size;    
    } catch (const std::exception &e) {
        return Status(e.what());
    }

    return Status::OK();
}

void ServerEvent::copy_events_arr(size_t src_size, const epoll_event *src, epoll_event *dst) {
    for (size_t i = 0; i < src_size; ++i) {
        dst[i] = src[i];
    }
}

Status ServerEvent::event_mod(uint32_t events, int event_fd) {
    int error;
    epoll_event event;

    event.data.fd = event_fd;
    event.events = events;

    error = epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, event_fd, &event);
    if (error < 0) {
        return Status(strerror(errno));
    }

    return Status::OK();
}
