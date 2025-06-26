#include "Parser/Parser.hpp"
#include "Server/Server.hpp"

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

  Server server(config);
	server.launch();
}