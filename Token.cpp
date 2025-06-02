#include "Token.hpp"

Token::Token(e_TokenType type, std::string lexeme, int line) : _type(type), _lexeme(lexeme), _line(line) {
}

std::string Token::getAll() {
	return _lexeme;
}

t_TokenType Token::getType() {
	return _type;
}

int Token::getLine() {
	return _line;
}