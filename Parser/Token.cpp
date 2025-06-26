#include "Token.hpp"

Token::Token(e_TokenType type, std::string lexeme, int line, int column) : _type(type), _lexeme(lexeme), _line(line), _column(column) {
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

int Token::getColumn() {
	return _column;
}

int Token::getLength() {
	return _lexeme.size();
}