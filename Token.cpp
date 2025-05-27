#include "Token.hpp"

Token::Token(e_TokenType type, std::string lexeme) : _type(type), _lexeme(lexeme) {
}

std::string Token::getAll()
{
	return _lexeme;
}