#ifndef PARSER_PARSER_HPP
#define PARSER_PARSER_HPP

#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Token.hpp"
#include "../Webserv.hpp"

class Parser {
   private:
	std::string &m_fileName;
	std::vector<Token> _tokens;
	std::vector<t_config> _configVector;
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
	void parseListen();
	void parseServerName();
	std::string parsePath();
	std::vector<std::string> parseIndex();
	t_methods parseMethods();
	t_location parseLocation();
	bool parseAutoIndex();
	std::map<int, std::string> parseErrorPage();
	bool isSpaceBetween(Token curr, Token next);
	std::vector<t_config> getConfigStruct();
	void fillDefaultValues();
	void parseReturn(std::string &path, int *code);

	t_config tempConfig;
};

#endif