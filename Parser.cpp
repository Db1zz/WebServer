#include "Parser.hpp"

Parser::Parser(std::string fileName) : m_fileName(fileName) {
	// check for right filetype, otherwise throw error
	try {
		std::cout << "[LOG]: Got filename as: " << m_fileName << "\n";
		if (access(m_fileName.c_str(), R_OK) == -1 || strstr(m_fileName.c_str(), ".conf\0") == NULL)
			throw std::logic_error("No file found with the extension '.conf' or no rights to read!");

	} catch (const std::exception &e) {
		std::cerr << e.what() << '\n';
		exit(1);
	}
}

Parser::Parser(const Parser &original) : m_fileName(original.m_fileName) {
	if (this != &original) {
	}
}

Parser &Parser::operator=(const Parser &original) {
	if (this != &original) {
		m_fileName = original.m_fileName;
	}
	return *this;
}

Parser::~Parser() {
}
