#include <string>
#include <vector>

#include "Parser/Parser.hpp"
#include "Server/Server.hpp"
#include "ServerLogger.hpp"
#include "colors.hpp"
#include "fs.hpp"

int main(int argc, char** argv) {
	std::string fileName;

	if (argc > 2) {
		std::cerr << "Incorrect use!\nCorrect use is: " << argv[0] << " [optional: config.conf]\n";
		return 2;
	} else if (argc == 1) {
		fileName = "default.conf";
	} else if (argc == 2) {
		fileName = argv[argc - 1];
	}
	std::vector<t_config> configs;
	try {
		Parser parser(fileName.c_str());
		configs = parser.getConfigStruct();
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
		ServerLogger server_logger("./Logs/");
		Status status = server_logger.init();
		if (!status) {
			std::cout << "[ServerLogger] " << RED300 << "Fatal Error: " << RESET << status.msg()
					  << std::endl;
			return 1;
		}
		Server server(configs, server_logger);
		status = server.launch();
		if (!status) {
			std::cout << "[Server] " << RED300 << "Fatal Error: " << RESET << status.msg()
					  << std::endl;
			return 1;
		}

	int epoll_fd = epoll_create(1);
	int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);

	epoll_event new_event;
	new_event.events = EPOLLIN | EPOLLOUT;
	new_event.data.fd = timer_fd;

	struct timespec start;
	struct itimerspec interval;

	const int TIMEOUT_CONST_TIME = 5;

	clock_gettime(CLOCK_MONOTONIC, &start);

	interval.it_value.tv_sec = start.tv_sec + TIMEOUT_CONST_TIME;
	interval.it_value.tv_nsec = 0;
	interval.it_interval.tv_sec = TIMEOUT_CONST_TIME;
	interval.it_interval.tv_nsec = 0;

	size_t events_size = 1000;
	epoll_event events[1000];

	timerfd_settime(timer_fd, TFD_TIMER_ABSTIME, &interval, NULL);
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &new_event);

	while (true) {
		int amount_of_events = epoll_wait(epoll_fd, events, events_size, -1);
		if (amount_of_events > 0 ) {
			// size_t buff_size = 100000;
			// char buff[buff_size];
			// int rd_bytes = read(events[0].data.fd, buff, buff_size);
	
			// buff[rd_bytes] = 0;
			// if (rd_bytes > 0) {
			// 	std::cout << "Elapsed time: " << buff << std::endl;
			// }

			// to update a time we have to set it again?
			clock_gettime(CLOCK_MONOTONIC, &start);
			interval.it_value.tv_sec = start.tv_sec + TIMEOUT_CONST_TIME + 5;
			interval.it_interval.tv_sec = TIMEOUT_CONST_TIME + 5;
			timerfd_settime(timer_fd, TFD_TIMER_ABSTIME, &interval, NULL);
			std::cout << "Timer updated!\n";
		}
	}
	
	return 0;
}