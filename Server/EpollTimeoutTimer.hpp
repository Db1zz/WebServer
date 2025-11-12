#ifndef SERVER_EPOLL_TIMEOUT_TIMER_HPP_
#define SERVER_EPOLL_TIMEOUT_TIMER_HPP_

#include <sys/timerfd.h>

#include "FileDescriptor.hpp"
#include "ITimeoutTimer.hpp"
#include "ServerEvent.hpp"
#include "Exceptions/SystemException.hpp"

class EpollTimeoutTimer : public ITimeoutTimer {
   public:
	EpollTimeoutTimer(ServerEvent* server_event, IEventContext* event_context, time_t timeout_time)
		: _server_event(server_event),
		  _event_context(event_context),
		  _timeout_time(timeout_time),
		  _timer_fd(),
		  _is_started(false) {}

	~EpollTimeoutTimer() { 
		stop();
	}

	void start() {
		if (_is_started == true) {
			return;
		}

		clock_gettime(CLOCK_MONOTONIC, &_start_time);

		_interval_time.it_value.tv_sec = _start_time.tv_sec + _timeout_time;
		_interval_time.it_value.tv_nsec = 0;
		_interval_time.it_interval.tv_sec = _timeout_time;
		_interval_time.it_interval.tv_nsec = 0;

		int timerfd = timerfd_create(CLOCK_REALTIME, 0);
		if (timerfd < 0) {
			throw SystemException(LOG_INFO(), "timerfd_create*()" + std::string(strerror(errno)));
		}

		_timer_fd.set_fd(timerfd);
		timerfd_settime(_timer_fd.get_fd(), TFD_TIMER_ABSTIME, &_interval_time, NULL);
		_server_event->register_event(EPOLLIN, _timer_fd.get_fd(), _event_context);
		_is_started = true;
	}

	void stop() {
		if (_is_started == false) {
			return;
		}

		_server_event->unregister_event(_timer_fd.get_fd());
		_timer_fd.close_fd();
		_is_started = false;
	}

	void reset() {
		clock_gettime(CLOCK_MONOTONIC, &_start_time);
		_interval_time.it_value.tv_sec = _start_time.tv_sec + _timeout_time;
		_interval_time.it_value.tv_nsec = 0;
		_interval_time.it_interval.tv_sec = _timeout_time;
		_interval_time.it_interval.tv_nsec = 0;
		timerfd_settime(_timer_fd.get_fd(), TFD_TIMER_ABSTIME, &_interval_time, NULL);
	}

	bool is_expired() const {
		struct timespec curr_time;
		clock_gettime(CLOCK_MONOTONIC, &curr_time);
		return curr_time.tv_sec - _start_time.tv_sec > _timeout_time;
	}

   private:
	ServerEvent* _server_event;
	IEventContext* _event_context;
	time_t _timeout_time;
	struct timespec _start_time;
	struct itimerspec _interval_time;
	FileDescriptor _timer_fd;
	bool _is_started;
};

#endif // SERVER_EPOLL_TIMEOUT_TIMER_HPP_