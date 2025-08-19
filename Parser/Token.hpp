#ifndef PARSER_TOKEN_HPP
#define PARSER_TOKEN_HPP

#include <string>

typedef enum e_TokenType {
	LEFT_BRACE,
	RIGHT_BRACE,
	DOT,
	MINUS,
	SLASH,
	SEMICOLON,
	COLON,

	IDENTIFIER,
	SERVER,
	LISTEN,
	HOST,
	SERVER_NAME,
	METHODS,
	GET,
	POST,
	DELETE,
	AUTO_INDEX,
	ON,
	OFF,
	INDEX,
	ROOT,
	ERROR_PAGE,
	LOCATION,
	CGI,
	MAX_CLIENT_BODY_SIZE,
	RETURN,
	chunked_transfer_encoding,
	chunked_threshold,
	chunked_size,

	END_OF_FILE,
} t_TokenType;

class Token {
   private:
	e_TokenType _type;
	std::string _lexeme;
	int _line;
	int _column;

   public:
	Token(e_TokenType type, std::string lexeme, int line, int colon);
	std::string getAll();
	t_TokenType getType();
	int getLine();
	int getColumn();
	int getLength();
};

#endif