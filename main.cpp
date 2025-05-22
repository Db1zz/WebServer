#include "Sockets/ASocket.hpp"
#include "Sockets/ClientSocket.hpp"
#include "Sockets/ServerSocket.hpp"
#include "Sockets/ListeningSocket.hpp"

int main() {
	try {
		std::cout << CYAN300 <<"start..." << RESET << std::endl;
		ServerSocket bs(AF_INET, SOCK_STREAM, 0, 81, INADDR_ANY);
		std::cout << "binding socket...done" << std::endl;
		ListeningSocket ls(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY, 10);
		std::cout << "listening socket...done" << std::endl;
	}
	catch(std::exception &e) {
		std::cerr << RED300 << e.what() << RESET << std::endl;
	}
	std::cout << "yahoo!" << std::endl;

}