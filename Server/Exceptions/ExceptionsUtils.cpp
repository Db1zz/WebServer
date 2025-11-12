#include "ExceptionsUtils.hpp"

#include <sstream>

namespace exception_utils {
std::string itoa(int num) {
	std::ostringstream os;
	os << num;
	return os.str();
}
} // namespace exception_utils