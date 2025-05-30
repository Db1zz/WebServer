#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <string>

typedef struct s_methods {
	bool m_get;
	bool m_post;
	bool m_delete;
} t_methods;

typedef struct s_location {
} t_location;

typedef struct s_config {
	std::vector<std::string> server_name;
	std::vector<int> listen;
	// int host;
	t_methods methods;
	bool auto_index;  // false by default
	std::string root;
	std::vector<std::string> index;
	std::map<int, std::string> error_page;
	std::vector<t_location> location;
	std::string max_client_body;
} t_config;

typedef enum e_TokenType {
	LEFT_BRACE,
	RIGHT_BRACE,
	DOT,
	MINUS,
	SLASH,
	SEMICOLON,
	COLON,

	IDENTIFIER,
	STRING,
	NUMBER,
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
	CGI,
	MAX_CLIENT_BODY_SIZE,
	RETURN,

	END_OF_FILE,
} t_TokenType;

#endif