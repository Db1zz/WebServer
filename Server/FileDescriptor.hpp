#ifndef SERVER_FILE_DESCRIPTOR_HPP_
#define SERVER_FILE_DESCRIPTOR_HPP_

#include "status.hpp"

#include <unistd.h>
#include <errno.h>
#include <string.h>

class FileDescriptor {
   public:
	enum Type { NoType, SocketFD, CGIFD };

	FileDescriptor(Type fd_type, int fd) : _fd_type(fd_type), _fd(fd) {}
	FileDescriptor() : _fd(-1) {}
	
	void close_fd() {
		if (_fd >= 0) {
			close(_fd);
			_fd = -1;
		}
	}

	void set_fd(int fd) {
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