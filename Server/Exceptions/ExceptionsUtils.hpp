#ifndef SERVER_EXCEPTION_EXCEPTIONS_UTILS_HPP_
#define SERVER_EXCEPTION_EXCEPTIONS_UTILS_HPP_

#include <string>

namespace exception_utils {
std::string itoa(int num);

#define LOG_INFO()                                                                          \
	std::string("file ") + __FILE__ + " in function " + __PRETTY_FUNCTION__ + " on line " + \
		exception_utils::itoa(__LINE__)

} // namespace exception_utils

#endif // SERVER_EXCEPTION_EXCEPTIONS_UTILS_HPP_
