#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

#include "Webserv.hpp"

class Token {
   private:
	e_TokenType _type;
	std::string _lexeme;

   public:
	Token(e_TokenType type, std::string lexeme);
	std::string getAll();
};

#endif