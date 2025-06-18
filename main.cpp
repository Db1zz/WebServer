#include "Parser/Parser.hpp"
#include "Sockets/ClientSocket.hpp"
#include "Sockets/ServerSocket.hpp"
#include "Sockets/ASocket.hpp"
#include "Server/Server.hpp"

int main(int argc, char** argv) {
	std::string fileName;

	if (argc > 2) {
		std::cerr << "Incorrect use!\nCorrect use is: " << argv[0]
				  << " [optional: config.conf]\n";
		return 2;
	} else if (argc == 1) {
		fileName = "default.conf";
	} else if (argc == 2) {
		fileName = argv[argc - 1];
	}
	Parser parser(fileName.c_str());
	parser.getConfigStruct();
	Server server;
	server.get_socket()->set_opt(SO_REUSEADDR, true);
	server.launch();
}