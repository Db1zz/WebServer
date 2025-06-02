#ifndef PARSER_HPP
#define PARSER_HPP

#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>

#include "Token.hpp"
#include "Webserv.hpp"

class Parser {
   private:
	std::string &m_fileName;
	std::vector<Token> _tokens;
	std::map<std::string, t_TokenType> _keywords;
	std::string _source;
	int _start;
	size_t _current;
	int _line;
	int _currentToken;

   public:
	Parser(std::string fileName);
	Parser(const Parser &original);
	Parser &operator=(const Parser &original);
	~Parser();
	std::vector<Token> scanTokens();
	void addKeywords();
	bool isAtEnd();
	void addToken(t_TokenType type);
	char advance();
	char peek();
	void identifier(char c);
	void scanToken();
	void parseConfig();
	bool tokenIsAtEnd();
	bool match(t_TokenType type);
	Token tokenAdvance();
	Token previous();
	Token tokenPeek();
	bool check(t_TokenType);
	Token consume(t_TokenType type, std::string message);

};

#endif