#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <string>

typedef struct s_config {
	std::string server_name;
	int listen;
	int host;
	std::string methods;
	bool auto_index;
	std::string max_client_body;
} t_config;

typedef enum e_TokenType {
	LEFT_BRACE,
	RIGHT_BRACE,
	DOT,
	MINUS,
	SLASH,

	IDENTIFIER,
	STRING,
	NUMBER,

	ON,
	OFF,
	RETURN,

	END_OF_FILE,
} t_TokenType;

#endif