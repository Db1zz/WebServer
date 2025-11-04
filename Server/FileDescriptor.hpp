#ifndef SERVER_FILE_DESCRIPTOR_HPP_
#define SERVER_FILE_DESCRIPTOR_HPP_

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "status.hpp"
#include "timer.hpp"

class FileDescriptor {
   public:
	enum Type { NoType, SocketFD, CGIFD };

	FileDescriptor(Type fd_type, int fd, std::time_t idle_time)
		: _start(timer::now()), _idle_time(idle_time), _fd_type(fd_type), _fd(fd) {}
	FileDescriptor(Type fd_type, int fd)
		: _start(timer::now()), _idle_time(-1), _fd_type(fd_type), _fd(fd) {}
	FileDescriptor() : _start(timer::now()), _idle_time(-1), _fd_type(NoType), _fd(-1) {}
	virtual ~FileDescriptor() { close_fd(); }

	void close_fd() {
		if (_fd >= 0) {
			close(_fd);
			_fd = -1;
		}
	}

	void set_fd(int fd) { _fd = fd; }

	int get_fd() const { return _fd; }

	void set_fd_type(Type fd_type) { _fd_type = fd_type; }
	Type get_fd_type() const { return _fd_type; }
	std::time_t get_start_time() { return _start; }
	std::time_t get_idle_time() { return _idle_time; }
	void set_idle_time(std::time_t idle_time) { _idle_time = idle_time; }

   protected:
	std::time_t _start;
	std::time_t _idle_time;
	Type _fd_type;
	int _fd;
};

#endif // SERVER_FILE_DESCRIPTOR_HPP_