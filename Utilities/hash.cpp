#include "hash.hpp"
#include <ctime>
#include <cstdlib>
#include <unistd.h>

std::string HashGenerator::generate(const int len) {
	static bool seeded = false;
	if (!seeded) {
		srand(static_cast<unsigned>(time(NULL) * getpid()));
		seeded = true;
	}
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	std::string temp_string;
	temp_string.reserve(len);

	for (int i = 0; i < len; ++i) {
		temp_string += alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	return temp_string;
}
