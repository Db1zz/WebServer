#include <string>
#include <vector>

#include "Parser/Parser.hpp"
#include "Server/Server.hpp"
#include "Server/ServerRequestParser.hpp"
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

	// std::ostringstream oss;

	// oss << "POST /upload HTTP/1.1\r\n";
	// oss << "Host: example.com\r\n";
	// oss << "Content-Type: multipart/form-data; boundary=----Boundary123\r\n";
	// oss << "Content-Length: 1024200\r\n";
	// oss << "\r\n";

	// oss << "------Boundary123\r\n";
	// oss << "Content-Disposition: form-data; name=\"file\"; filename=\"test.bin\"\r\n";
	// oss << "Content-Type: application/octet-stream\r\n";
	// oss << "\r\n";

	// for (int i = 0; i < 1024 * 1000; i++) {
	// 	oss << "A";
	// }

	// oss << "\r\n";
	// oss << "------Boundary123--\r\n";

	// std::string request_string = oss.str();

	// ServerLogger server_logger("./Logs/");
	// Status status = server_logger.init();

	// t_request request;
	// request.boundary = "------geckoformboundarydbe55acddb6231da461629742a34e1e6";

	// ServerRequestParser::parse_request_header(request_string, request, server_logger);

	// std::string buff;
	// const size_t chunk_size = 1024;
	// size_t readed = 0;

	// while (buff.size() != 1024 * 1000) {
	// 	request.cache.append(request_string.substr(readed, chunk_size));
	// 	ServerRequestParser::parse_request_body_chunk(request);
	// 	if (!request.body_chunk.empty()) {
	// 		buff.append(request.body_chunk);
	// 		readed += chunk_size;
	// 	}
	// 	std::cout << "get!\n";
	// }

	// std::cout << "buffer_size: " << buff.size() << std::endl;
	// std::cout << buff << std::endl;

	// std::cout << "Done!\n";

	std::vector<t_config> config;
	try {
		Parser parser(fileName.c_str());
		config = parser.getConfigStruct();

	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
	try {
		ServerLogger server_logger("./Logs/");
		Status status = server_logger.init();
		if (!status) {
			std::cout << "[ServerLogger] " << RED300 << "Fatal Error: " << RESET << status.msg()
					  << std::endl;
			return 1;
		}
		Server server(config, server_logger);
		status = server.launch();
		if (!status) {
			std::cout << "[Server] " << RED300 << "Fatal Error: " << RESET << status.msg()
					  << std::endl;
			return 1;
		}
	} catch (const std::exception& e) {
		std::cout << "[Server] " << RED300 << "Fatal Error: " << RESET << e.what() << std::endl;
		return 1;
	}
	return 0;
}