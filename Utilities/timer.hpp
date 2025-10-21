#ifndef UTILITIES_TIMER_HPP_
#define UTILITIES_TIMER_HPP_

#include <ctime>

namespace timer {
std::time_t diff(std::time_t start, std::time_t curr);
std::time_t now();
} // namespace timer

#endif // UTILITIES_TIMER_HPP_