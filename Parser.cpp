#include "Parser.hpp"

Parser::Parser(std::string fileName) : m_fileName(fileName) {
	_start = 0;
	_current = 0;
	_line = 1;
	_currentToken = 0;
	std::cout << "[LOG]: Got filename as: " << m_fileName << "\n";
	if (access(m_fileName.c_str(), R_OK) == -1 || strstr(m_fileName.c_str(), ".conf\0") == NULL) {
		std::cerr << "No file found with the extension '.conf' or no rights to read!\n";
		exit(1);
	}
	std::ifstream configFile(m_fileName.c_str());
	if (!configFile.is_open()) {
		std::cerr << "Failed to open file: " << m_fileName << "\n";
		exit(1);
	}
	std::string tempString;
	while (std::getline(configFile, tempString)) {
		_source += tempString;
		_source += '\n';
	}
	addKeywords();
	std::vector<Token> temp = scanTokens();
	for (size_t i = 0; i < temp.size(); i++) {
		if (temp[i].getType() != END_OF_FILE)
			std::cout << temp[i].getAll() << "\n";
	}
	std::cout << "--------------------------------\n";
	parseConfig();
}

Parser::Parser(const Parser &original) : m_fileName(original.m_fileName) {
	if (this != &original) {
	}
}

Parser &Parser::operator=(const Parser &original) {
	if (this != &original) {
		m_fileName = original.m_fileName;
	}
	return *this;
}

Parser::~Parser() {
}

void Parser::addKeywords() {
	_keywords["server"] = SERVER;
	_keywords["listen"] = LISTEN;
	_keywords["host"] = HOST;
	_keywords["server_name"] = SERVER_NAME;
	_keywords["methods"] = METHODS;
	_keywords["get"] = GET;
	_keywords["post"] = POST;
	_keywords["delete"] = DELETE;
	_keywords["auto_index"] = AUTO_INDEX;
	_keywords["on"] = ON;
	_keywords["off"] = OFF;
	_keywords["index"] = INDEX;
	_keywords["root"] = ROOT;
	_keywords["error_page"] = ERROR_PAGE;
	_keywords["cgi"] = CGI;
	_keywords["max_client_body_size"] = MAX_CLIENT_BODY_SIZE;
	_keywords["return"] = RETURN;
}

std::vector<Token> Parser::scanTokens() {
	while (_current < _source.size()) {
		_start = _current;
		scanToken();
	}
	_tokens.push_back(Token(END_OF_FILE, "", -1));
	return _tokens;
}

bool Parser::isAtEnd() {
	return _current >= _source.length();
}

void Parser::addToken(t_TokenType type) {
	std::string text = _source.substr(_start, _current - _start);
	_tokens.push_back(Token(type, text, _line));
}

char Parser::advance() {
	_current++;
	return _source.at(_current - 1);
}

char Parser::peek() {
	return _source.at(_current);
}

void Parser::identifier(char c) {
	while (!isAtEnd()) {
		c = peek();
		if (isalnum(c) || c == '_' || c == '-')
			advance();
		else
			break;
	}
	std::string text = _source.substr(_start, _current - _start);
	std::map<std::string, t_TokenType>::iterator it = _keywords.find(text);
	if (it != _keywords.end()) {
		addToken(it->second);
	} else {
		addToken(IDENTIFIER);
	}
}

void Parser::scanToken() {
	char c = advance();
	switch (c) {
		case ' ':
		case '\r':
		case '\t':
			break;
		case '{':
			addToken(LEFT_BRACE);
			break;
		case '}':
			addToken(RIGHT_BRACE);
			break;
		case '.':
			addToken(DOT);
			break;
		case '-':
			addToken(MINUS);
			break;
		case '/':
			addToken(SLASH);
			break;
		case '#':
			while (_current < _source.size() && _source[_current] != '\n')
				_current++;
			break;
		case '\n':
			_line++;
			break;
		case ':':
			addToken(COLON);
			break;
		case ';':
			addToken(SEMICOLON);
			break;
		default:
			if (isalnum(c) || c == '_' || c == '-') {
				identifier(c);
			} else {
				std::cerr << "Unexpected character '" << c << "' at line: " << _line << '\n';
			}
	}
}

bool Parser::tokenIsAtEnd() {
	if (_tokens.at(_currentToken).getType() == END_OF_FILE)
		return true;
	return false;
}

Token Parser::tokenAdvance() {
	if (!tokenIsAtEnd())
		_currentToken++;
	return previous();
}

bool Parser::check(t_TokenType type) {
	if (tokenIsAtEnd())
		return false;
	return tokenPeek().getType() == type;
}

Token Parser::tokenPeek() {
	return _tokens.at(_currentToken);
}

Token Parser::previous() {
	return _tokens.at(_currentToken - 1);
}

bool Parser::match(t_TokenType type) {
	if (_tokens.at(_currentToken).getType() == type) {
		tokenAdvance();
		return true;
	}
	return false;
}

Token Parser::consume(t_TokenType type, std::string message) {
	if (check(type))
		return tokenAdvance();
	std::stringstream str;
	str << "Parse error at line " << tokenPeek().getLine() << ": " << message << " got '" << _tokens.at(_currentToken).getAll() << "' instead\n";
	throw std::runtime_error(str.str());
}

void Parser::parseListen() {
	// string stream and than give back the string in the config
	std::stringstream str;
	if (match(IDENTIFIER)) {
		str << previous().getAll();
		if (match(SEMICOLON)) {
			// its only a port needs to be added to the config
			return;
		}
		for (size_t i = 0; i < 3; i++) {
			if (!match(DOT))
				throw std::runtime_error("Expexted a dot between ip adress numbers");
			str << previous().getAll();
			if (!match(IDENTIFIER))
				throw std::runtime_error("Expected a number after the dot ");
			str << previous().getAll();
		}
		// so far ip address add it to the config
		if (check(COLON)) {
			consume(COLON, "expected ':' for the port part");
			consume(IDENTIFIER, "expected port after colon");
			str << previous().getAll();
			// add the port to the config
		} else
			consume(SEMICOLON, "expected ';' after the statement");
		std::cout << str.str() << '\n';
	} else
		throw std::runtime_error("Expected identifier");
}

void Parser::parseRoot() {
	if (!check(IDENTIFIER))
		throw std::runtime_error("Expected identifier");
	consume(SEMICOLON, "expected ';' after the statement");
}

void Parser::parseServerName() {
	consume(IDENTIFIER, "Expected IDENTIFIER");
	tempConfig.server_name.push_back(previous().getAll());
	int i = 0;
	while (check(IDENTIFIER)) {
		consume(IDENTIFIER, "expected identifier");
		tempConfig.server_name.push_back(previous().getAll());
		i++;
	}
	consume(SEMICOLON, "expected ';' after the statement");
}

void Parser::parseConfig() {
	while (!tokenIsAtEnd()) {
		if (match(SERVER)) {
			consume(LEFT_BRACE, "expected '{' after server block");
			int i = 0;	// DEBUG ONLY
			while (!check(RIGHT_BRACE) && !tokenIsAtEnd()) {
				if (match(LISTEN)) {
					parseListen();
				} else if (match(SERVER_NAME)) {
					parseServerName();
				}
				i++;		  // DEBUG
				if (i > 2)	  // DEBUG
				{
					for (size_t i = 0; i < tempConfig.server_name.size(); i++)
					{
						std::cout << "server name: " << tempConfig.server_name.at(i) << '\n';
					}
					exit(1);  // DEBUG escape loop manually for now
				}
			}
			consume(RIGHT_BRACE, "expected terminator '}' after server block content");
		} else {
			std::cerr << "Expected server block\n";
			exit(1);
		}
	}
	// for (size_t i = 0; i < tempConfig.server_name.size(); i++)
	// {
	// 	std::cout << i << '\n';
	// }
}