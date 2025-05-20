#ifndef PARSER_HPP
#define PARSER_HPP

#include <exception>
#include <fstream>
#include <iostream>

class Parser {
	private:
		std::string &m_fileName;
	public:
		Parser(std::string fileName);
		Parser(const Parser &original);
		Parser &operator=(const Parser &original);
		~Parser();
};

#endif