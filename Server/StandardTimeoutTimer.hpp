#ifndef SERVER_SERVER_STANDARD_TIMEOUT_TIMER_HPP_
#define SERVER_SERVER_STANDARD_TIMEOUT_TIMER_HPP_

#include <ctime>

#include "ITimeoutTimer.hpp"

class StandardTimeoutTimer : public ITimeoutTimer {
   public:
	StandardTimeoutTimer(std::time_t timeout_in_seconds)
		: _is_timer_started(false), _timeout_in_seconds(timeout_in_seconds) {
		_start_time = std::time(NULL);
	}

	void start() {
		if (_is_timer_started == true) {
			return;
		}

		_start_time = std::time(NULL);

		_is_timer_started = true;
	}

	void stop() {
		if (_is_timer_started == false) {
			return;
		}

		_is_timer_started = false;
	}

	void reset() {
		stop();
		start();
	}

	bool is_expired() const {
		std::time_t curr_time = std::time(NULL);

		return (curr_time - _start_time > _timeout_in_seconds);
	}

   private:
	bool _is_timer_started;
	std::time_t _start_time;
	const std::time_t _timeout_in_seconds;
};

#endif // SERVER_SERVER_STANDARD_TIMEOUT_TIMER_HPP_