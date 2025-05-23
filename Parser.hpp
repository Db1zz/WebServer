#ifndef PARSER_HPP
#define PARSER_HPP

#include <exception>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include "Webserv.hpp"

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