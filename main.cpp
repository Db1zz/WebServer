#include "Parser/Parser.hpp"
#include "Server/Server.hpp"
#include "colors.hpp"

#include <vector>
#include <string>

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

	std::vector<t_config> config;
	try
	{
		Parser parser(fileName.c_str());
		config = parser.getConfigStruct();
		
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	try {
		Server server(config);
		server.launch();
	} catch (const std::exception &e) {
		std::cout << "[Server] " << RED300 << "Fatal Error: " << RESET << e.what() << std::endl;
		return 1;
	}
	return 0;
}