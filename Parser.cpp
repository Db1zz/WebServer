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
	_keywords["GET"] = GET;
	_keywords["POST"] = POST;
	_keywords["DELETE"] = DELETE;
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
	_tokens.push_back(Token(END_OF_FILE, "", -1, -1));
	return _tokens;
}

bool Parser::isAtEnd() {
	return _current >= _source.length();
}

void Parser::addToken(t_TokenType type) {
	std::string text = _source.substr(_start, _current - _start);
	_tokens.push_back(Token(type, text, _line, _start));
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
	while (!check(SEMICOLON)) {
		std::stringstream str;
		bool is_ip = false;
		int dotCount = 0;
		if (match(IDENTIFIER)) {
			std::string first = previous().getAll();
			str << first;
			while (check(DOT) && dotCount < 3) {
				is_ip = true;
				consume(DOT, "expected dot between IP address numbers");
				str << ".";
				consume(IDENTIFIER, "expected number after dot");
				str << previous().getAll();
				dotCount++;
			}
			if (check(COLON)) {
				consume(COLON, "expected ':' for the port part");
				consume(IDENTIFIER, "expected port after colon");
				str << ":" << previous().getAll();
				tempConfig.host.push_back(str.str());
			} else if (is_ip) {
				tempConfig.host.push_back(str.str());
			} else {
				tempConfig.port.push_back(str.str());
			}
		} else {
			throw std::runtime_error("Expected identifier in listen directive");
		}
	}
	consume(SEMICOLON, "expected ';' after the statement");
}

void Parser::parseIndex() {
	while (!check(SEMICOLON)) {
		std::stringstream temp;
		int lastEnd = -1;
		if (check(DOT)) {
			Token firstDot = consume(DOT, "expected dot at start of filename");
			temp << firstDot.getAll();
			lastEnd = firstDot.getColon() + firstDot.getAll().length();
			Token first = consume(IDENTIFIER, "expected an identifier");
			temp << first.getAll();
			lastEnd = first.getColon() + first.getAll().length();
		} else {
			Token first = consume(IDENTIFIER, "expected an identifier");
			temp << first.getAll();
			lastEnd = first.getColon() + first.getAll().length();
		}
		while (check(DOT)) {
			Token dot = tokenPeek();
			if (dot.getColon() != lastEnd) break;
			temp << consume(DOT, "expected dot").getAll();
			Token next = consume(IDENTIFIER, "expected identifier after dot");
			temp << next.getAll();
			lastEnd = next.getColon() + next.getAll().length();
		}
		tempConfig.common.index.push_back(temp.str());
	}
	consume(SEMICOLON, "expected ';' after the statement");
}

std::string Parser::parsePath() {
	std::stringstream temp;
	// Accept any sequence of SLASH and/or IDENTIFIER tokens, in any order
	bool found = false;
	while (check(IDENTIFIER) || check(SLASH)) {
		if (check(SLASH)) {
			temp << consume(SLASH, "expected a '/'").getAll();
			found = true;
		} else if (check(IDENTIFIER)) {
			temp << consume(IDENTIFIER, "expected an identifier").getAll();
			found = true;
		}
	}
	if (!found)
		throw std::runtime_error("Expected at least one '/' or identifier in root path");
	return temp.str();
}

void Parser::parseLocation() {
	t_location tempLocation;
	tempLocation.path = parsePath();
	consume(SEMICOLON, "expected a ';' after the statement");
	// parse the common things
	tempConfig.location.push_back(tempLocation);
}

void Parser::parseServerName() {
	bool found = false;
	while (check(IDENTIFIER)) {
		std::stringstream temp;
		temp << consume(IDENTIFIER, "expected an identifier").getAll();
		while (check(DOT)) {
			temp << consume(DOT, "expected dot").getAll();
			temp << consume(IDENTIFIER, "expected identifier after dot").getAll();
		}
		tempConfig.server_name.push_back(temp.str());
		found = true;
	}
	if (!found)
		throw std::runtime_error("Expected at least one server name");
	consume(SEMICOLON, "expected ';' after the statement");
}

void Parser::parseMethods() {
	while (!check(SEMICOLON)) {
		std::cout << _tokens.at(_currentToken).getType() << '\n';
		if (match(GET)) {
			tempConfig.common.methods.getMethod = true;
		} else if (match(POST)) {
			tempConfig.common.methods.postMethod = true;
		} else if (match(DELETE)) {
			tempConfig.common.methods.deleteMethod = true;
		} else {
			std::cout << previous().getAll() << '\n';
			throw std::runtime_error("Unrecognized method found. Allowed methods are GET POST DELETE");
		}
	}
	if (!tempConfig.common.methods.deleteMethod && !tempConfig.common.methods.getMethod && !tempConfig.common.methods.postMethod)
		throw std::runtime_error("No method found");
	consume(SEMICOLON, "exptected ';' after the statement");
}

void Parser::parseAutoIndex() {
	if (match(ON))
		tempConfig.common.auto_index = true;
	else if (check(OFF))
		consume(OFF, "expected off");
	else
		throw std::runtime_error("Unrecognized option. Allowed options are on or off!");
	consume(SEMICOLON, "exptected ';' after the statement");
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
				} else if (match(ROOT)) {
					tempConfig.common.root = parsePath();
					consume(SEMICOLON, "expected ';' after the statement");
				} else if (match(INDEX)) {
					parseIndex();
				} else if (match(METHODS)) {
					parseMethods();
				} else if (match(AUTO_INDEX)) {
					parseAutoIndex();
				} else {
					throw std::runtime_error("Unexpected token, got " + previous().getAll());
				}
				i++;		// DEBUG
				if (i > 2)	// DEBUG
				{
					for (size_t i = 0; i < tempConfig.common.index.size(); i++) {
						std::cout << tempConfig.common.index.at(i) << '\n';
					}
					std::cout << tempConfig.common.root << '\n';
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