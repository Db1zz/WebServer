#include "Token.hpp"

Token::Token(e_TokenType type, std::string lexeme, int line, int colon) : _type(type), _lexeme(lexeme), _line(line), _colon(colon) {
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

int Token::getColon() {
	return _colon;
}