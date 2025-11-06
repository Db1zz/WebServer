#ifndef SERVER_I_TIMEOUT_TIMER_HPP_
#define SERVER_I_TIMEOUT_TIMER_HPP_

class ITimeoutTimer {
public:
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void reset() = 0;
	virtual bool is_expired() const = 0;
	virtual ~ITimeoutTimer() {}
};

#endif // SERVER_I_TIMEOUT_TIMER_HPP_