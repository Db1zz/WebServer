#ifndef SERVER_FILE_DESCRIPTOR_HPP_
#define SERVER_FILE_DESCRIPTOR_HPP_

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

#include "Exceptions/SystemException.hpp"
#include "status.hpp"
#include "timer.hpp"

class FileDescriptor {
   public:
	enum Type { NoType, SocketFD, CGIFD };
	FileDescriptor(Type fd_type, int fd) : _fd_type(fd_type), _fd(fd) {}
	FileDescriptor() : _fd_type(NoType), _fd(-1) {}
	virtual ~FileDescriptor() { close_fd(); }

	void close_fd() {
		if (_fd >= 0) {
			close(_fd);
			_fd = -1;
		}
	}

	void set_nonblock() {
		if (_fd < 0) {
			return;
		}

		if (fcntl(_fd, F_SETFL, O_NONBLOCK) < 0) {
			throw SystemException(LOG_INFO(), strerror(errno));
		}
	}

	void set_fd(int fd) {
		close_fd();
		_fd = fd;
	}

	int get_fd() const { return _fd; }

	void set_fd_type(Type fd_type) { _fd_type = fd_type; }
	Type get_fd_type() const { return _fd_type; }

   protected:
	Type _fd_type;
	int _fd;
};

#endif // SERVER_FILE_DESCRIPTOR_HPP_