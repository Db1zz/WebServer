#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

#include "Webserv.hpp"

class Token {
   private:
	e_TokenType _type;
	std::string _lexeme;
	int _line;

   public:
	Token(e_TokenType type, std::string lexeme, int line);
	std::string getAll();
	t_TokenType getType();
	int getLine();
};

#endif