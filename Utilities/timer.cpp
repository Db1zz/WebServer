#include "timer.hpp"

namespace timer {
std::time_t diff(std::time_t start, std::time_t curr) {
	return curr - start;
}

std::time_t now() {
	return std::time(NULL);
}
} // namespace timer