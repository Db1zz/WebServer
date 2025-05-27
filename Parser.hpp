#ifndef PARSER_HPP
#define PARSER_HPP

#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <vector>

#include "Token.hpp"
#include "Webserv.hpp"

class Parser {
   private:
	std::string &m_fileName;
	std::vector<Token> _tokens;
	std::string _source;
	int _start;
	size_t _current;
	int _line;

   public:
	Parser(std::string fileName);
	Parser(const Parser &original);
	Parser &operator=(const Parser &original);
	~Parser();
	std::vector<Token> scanTokens();
	bool isAtEnd();
	void addToken(t_TokenType type);
	char advance();
	void scanToken();
};

#endif