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
	/* for (size_t i = 0; i < temp.size(); i++) {
		if (temp[i].getType() != END_OF_FILE)
			std::cout << temp[i].getAll() << "\n";
	}
	std::cout << "--------------------------------\n"; */
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
	_keywords["location"] = LOCATION;
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
		if (isalnum(c) || c == '_' || c == '-' || c == '=' || c == '?' || c == '.' || c == '#' || c == '/' || c == '\'' || c == '\"')
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
			if (isalnum(c) || c == '_' || c == '-' || c == '=' || c == '?') {
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

bool Parser::isSpaceBetween(Token curr, Token next) {
	if (curr.getLength() + curr.getColon() + 1 == next.getColon()) {
		return true;
	}
	return false;
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
				for (size_t i = 0; i < tempConfig.host.size(); i++) {
					if (tempConfig.host.at(i) == str.str()) {
						throw std::runtime_error("Duplicate listen");
					}
				}
				tempConfig.host.push_back(str.str());
			} else if (is_ip) {
				for (size_t i = 0; i < tempConfig.host.size(); i++) {
					if (tempConfig.host.at(i) == str.str()) {
						throw std::runtime_error("Duplicate listen");
					}
				}
				tempConfig.host.push_back(str.str());
			} else {
				for (size_t i = 0; i < tempConfig.port.size(); i++) {
					if (tempConfig.port.at(i) == str.str()) {
						throw std::runtime_error("Duplicate listen");
					}
				}
				tempConfig.port.push_back(str.str());
			}
		} else {
			throw std::runtime_error("Expected identifier in listen directive");
		}
	}
	consume(SEMICOLON, "expected ';' after the statement");
}

std::vector<std::string> Parser::parseIndex() {
	std::vector<std::string> tempVector;
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
		tempVector.push_back(temp.str());
	}
	consume(SEMICOLON, "expected ';' after the statement");
	return tempVector;
}

std::string Parser::parsePath() {
	std::stringstream temp;
	while (check(IDENTIFIER) || check(SLASH) || check(DOT) || check(MINUS) || check(COLON)) {
		if (check(SLASH)) {
			temp << consume(SLASH, "expected a '/'").getAll();
		} else if (check(IDENTIFIER)) {
			temp << consume(IDENTIFIER, "expected an identifier").getAll();
		} else if (check(DOT)) {
			temp << consume(DOT, "expected a dot").getAll();
		} else if (check(MINUS)) {
			temp << consume(MINUS, "expected a minus").getAll();
		} else if (check(COLON)) {
			temp << consume(COLON, "expected a colon").getAll();
		}
	}
	return temp.str();
}

t_location Parser::parseLocation() {
	t_location tempLocation;
	t_commonConfig tempCommon;
	tempCommon.auto_index = false;
	tempCommon.max_client_body = 0;
	tempCommon.methods.deleteMethod = false;
	tempCommon.methods.getMethod = false;
	tempCommon.methods.postMethod = false;
	tempCommon.returnCode = -1;
	tempCommon.root = "";
	tempLocation.path = parsePath();
	tempCommon.cgi.clear();
	tempCommon.errorPage.clear();
	tempCommon.index.clear();
	consume(LEFT_BRACE, "expected opening '{' for location block");
	while (!check(RIGHT_BRACE)) {
		if (match(ROOT)) {
			tempCommon.root = parsePath();
			consume(SEMICOLON, "expected ';' after the statement");
		} else if (match(INDEX)) {
			std::vector<std::string> tempVector = parseIndex();
			tempCommon.index.insert(tempCommon.index.end(), tempVector.begin(), tempVector.end());
		} else if (match(METHODS)) {
			tempCommon.methods = parseMethods();
		} else if (match(AUTO_INDEX)) {
			tempCommon.auto_index = parseAutoIndex();
		} else if (match(ERROR_PAGE)) {
			std::map<int, std::string> tempMap = parseErrorPage();
			tempCommon.errorPage.insert(tempMap.begin(), tempMap.end());
		} else if (match(RETURN)) {
			parseReturn(tempCommon.returnPath, &tempCommon.returnCode);
		} else if (match(CGI)) {
			std::string extension;
			consume(DOT, "expected an extension type with a dot, example: .py");
			extension = "." + consume(IDENTIFIER, "expected an extension type with a dot, example: .py").getAll();
			tempCommon.cgi.insert(std::pair<std::string, std::string>(extension, parsePath()));
			consume(SEMICOLON, "expected ';' after the statement");
		} else if (match(MAX_CLIENT_BODY_SIZE)) {
			tempCommon.max_client_body = parseMaxClientBody();
			consume(SEMICOLON, "expected ';' after the statement");
		} else {
			throw std::runtime_error("Unexpected token in the location block: " + tokenPeek().getAll());
		}
	}
	consume(RIGHT_BRACE, "expected closing '}' for location block");
	tempLocation.common = tempCommon;
	return tempLocation;
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

t_methods Parser::parseMethods() {
	t_methods methodStruct;
	methodStruct.getMethod = false;
	methodStruct.postMethod = false;
	methodStruct.deleteMethod = false;
	while (!check(SEMICOLON)) {
		if (match(GET)) {
			methodStruct.getMethod = true;
		} else if (match(POST)) {
			methodStruct.postMethod = true;
		} else if (match(DELETE)) {
			methodStruct.deleteMethod = true;
		} else {
			throw std::runtime_error("Unrecognized method found. Allowed methods are GET POST DELETE");
		}
	}
	consume(SEMICOLON, "exptected ';' after the statement");
	return methodStruct;
}

bool Parser::parseAutoIndex() {
	bool autoIndex = false;
	if (match(ON))
		autoIndex = true;
	else if (check(OFF))
		consume(OFF, "expected off");
	else
		throw std::runtime_error("Unrecognized option. Allowed options are on or off!");
	consume(SEMICOLON, "exptected ';' after the statement");
	return autoIndex;
}

std::map<int, std::string> Parser::parseErrorPage() {
	std::vector<std::string> codes;
	std::string tempString;
	std::map<int, std::string> tempMap;
	bool nonDigit;
	while (check(IDENTIFIER)) {
		tempString = consume(IDENTIFIER, "expected identifier").getAll();
		if (!isSpaceBetween(previous(), tokenPeek()))
			break;
		for (size_t i = 0; i < tempString.size(); i++) {
			if (std::isdigit(tempString.at(i))) {
				nonDigit = false;
			} else {
				nonDigit = true;
				break;
			}
		}
		if (nonDigit)
			break;
		codes.push_back(tempString);
		tempString = "";
	}
	std::string path = tempString + parsePath();
	for (size_t i = 0; i < codes.size(); i++) {
		tempMap.insert(std::pair<int, std::string>(atoi(codes.at(i).c_str()), path));  // later maybe check if its correct error code
	}
	consume(SEMICOLON, "expected ';' after error_page");
	return tempMap;
}

std::vector<t_config> Parser::getConfigStruct() {
	std::cout << "-----------------------------\n";
	for (size_t i = 0; i < _configVector.size(); i++) {
		t_config temp = _configVector.at(i);

		// Print hosts
		for (size_t host_i = 0; host_i < temp.host.size(); host_i++)
			std::cout << "Host: " << temp.host.at(host_i) << '\n';

		// Print ports
		for (size_t port_i = 0; port_i < temp.port.size(); port_i++)
			std::cout << "Port: " << temp.port.at(port_i) << '\n';

		// Print server names
		for (size_t sn_i = 0; sn_i < temp.server_name.size(); sn_i++)
			std::cout << "Server Name: " << temp.server_name.at(sn_i) << '\n';

		// Print max client body size
		std::cout << "Max Client Body Size: " << temp.common.max_client_body << '\n';

		// Print common config
		std::cout << "Root: " << temp.common.root << '\n';
		std::cout << "Auto Index: " << (temp.common.auto_index ? "on" : "off") << '\n';
		std::cout << "Return Code: " << temp.common.returnCode << '\n';
		std::cout << "Return Path: " << temp.common.returnPath << '\n';

		// Print index files
		for (size_t idx = 0; idx < temp.common.index.size(); idx++)
			std::cout << "Index: " << temp.common.index.at(idx) << '\n';

		// Print error pages
		for (std::map<int, std::string>::iterator it = temp.common.errorPage.begin(); it != temp.common.errorPage.end(); ++it)
			std::cout << "Error Page: " << it->first << " -> " << it->second << '\n';

		// Print allowed methods
		std::cout << "Methods: "
				  << (temp.common.methods.getMethod ? "GET " : "")
				  << (temp.common.methods.postMethod ? "POST " : "")
				  << (temp.common.methods.deleteMethod ? "DELETE " : "")
				  << '\n';

		for (std::map<std::string, std::string>::const_iterator cgi_it = temp.common.cgi.begin(); cgi_it != temp.common.cgi.end(); cgi_it++)
			std::cout << "CGI: " << cgi_it->first << " " << cgi_it->second << '\n';
		// Print locations
		for (size_t loc_i = 0; loc_i < temp.location.size(); loc_i++) {
			const t_location &loc = temp.location.at(loc_i);
			std::cout << "Location Path: " << loc.path << '\n';
			std::cout << " Max Client Body Size: " << loc.common.max_client_body << '\n';
			std::cout << "  Root: " << loc.common.root << '\n';
			std::cout << "  Auto Index: " << (loc.common.auto_index ? "on" : "off") << '\n';
			std::cout << "  Return Code: " << loc.common.returnCode << '\n';
			std::cout << "  Return Path: " << loc.common.returnPath << '\n';
			for (size_t idx = 0; idx < loc.common.index.size(); idx++)
				std::cout << "  Index: " << loc.common.index.at(idx) << '\n';
			for (std::map<int, std::string>::const_iterator it = loc.common.errorPage.begin(); it != loc.common.errorPage.end(); ++it)
				std::cout << "  Error Page: " << it->first << " -> " << it->second << '\n';
			std::cout << "  Methods: "
					  << (loc.common.methods.getMethod ? "GET " : "")
					  << (loc.common.methods.postMethod ? "POST " : "")
					  << (loc.common.methods.deleteMethod ? "DELETE " : "")
					  << '\n';
			for (std::map<std::string, std::string>::const_iterator cgi_it = loc.common.cgi.begin(); cgi_it != loc.common.cgi.end(); cgi_it++)
				std::cout << "  CGI: " << cgi_it->first << " " << cgi_it->second << '\n';
		}
		std::cout << "-----------------------------\n";
	}
	return _configVector;
}

void Parser::fillDefaultValues() {
	tempConfig.common.max_client_body = 0;
	tempConfig.common.auto_index = false;
	tempConfig.host.clear();
	tempConfig.location.clear();
	tempConfig.port.clear();
	tempConfig.server_name.clear();
	tempConfig.common.cgi.clear();
	tempConfig.common.errorPage.clear();
	tempConfig.common.index.clear();
	tempConfig.common.methods.deleteMethod = false;
	tempConfig.common.methods.getMethod = false;
	tempConfig.common.methods.postMethod = false;
	tempConfig.common.returnCode = -1;
	tempConfig.common.root = "";
}

void Parser::parseReturn(std::string &path, int *code) {
	bool nonDigit = false;
	std::string tempString = tokenPeek().getAll();
	for (size_t i = 0; i < tempString.size(); i++) {
		if (std::isdigit(tempString.at(i))) {
			nonDigit = false;
		} else {
			nonDigit = true;
			break;
		}
	}
	if (!nonDigit)
		*code = atol(consume(IDENTIFIER, "expected a status code").getAll().c_str());
	if (nonDigit || !check(SEMICOLON))
		path = parsePath();
	consume(SEMICOLON, "expected ';' after the statement");
}

size_t Parser::parseMaxClientBody() {
	std::string number = consume(IDENTIFIER, "expected number and unit specifier").getAll();
	size_t i = 0;
	while (i < number.size()) {
		if (!std::isdigit(number.at(i)))
			break;
		i++;
	}
	if (i == number.size())
		return atol(number.c_str());
	switch (number.at(i)) {
		case 'k':
			return atol(number.c_str()) * 1024;
		case 'm':
			return atol(number.c_str()) * 1024 * 1024;
		case 'g':
			return atol(number.c_str()) * 1024 * 1024 * 1024;
		default:
			throw std::runtime_error("Unrecognizeable unit found in one of the max_client_body_size directive");
	}
	return 0;
}

bool Parser::does_port_exist(const std::string &port, const std::vector<std::string> &ports) {
	for (size_t i = 0; i < ports.size(); ++i) {
		if (ports[i] == port) {
			return true;
		}
	}
	return false;
}

void Parser::split_host_and_address() {
	t_config *config;
	size_t pos;
	std::string port;

	for (size_t i = 0; i < _configVector.size(); ++i) {
		config = &_configVector[i];
		for (size_t j = 0; j < config->host.size(); ++j) {
			pos = config->host[j].find(":");
			if (pos != std::string::npos) {
				port = config->host[j].substr(pos + 1);
				if (!does_port_exist(port, config->port)) {
					config->port.push_back(port);
				}
				config->host[j] = config->host[j].substr(0, pos);
			}
		}
	}
}

void Parser::parseConfig() {
	while (!tokenIsAtEnd()) {
		if (match(SERVER)) {
			fillDefaultValues();
			consume(LEFT_BRACE, "expected '{' after server block");
			while (!check(RIGHT_BRACE) && !tokenIsAtEnd()) {
				if (match(LISTEN)) {
					parseListen();
				} else if (match(SERVER_NAME)) {
					parseServerName();
				} else if (match(ROOT)) {
					tempConfig.common.root = parsePath();
					consume(SEMICOLON, "expected ';' after the statement");
				} else if (match(INDEX)) {
					std::vector<std::string> tempVector = parseIndex();
					tempConfig.common.index.insert(tempConfig.common.index.end(), tempVector.begin(), tempVector.end());
				} else if (match(METHODS)) {
					tempConfig.common.methods = parseMethods();
				} else if (match(AUTO_INDEX)) {
					tempConfig.common.auto_index = parseAutoIndex();
				} else if (match(LOCATION)) {
					tempConfig.location.push_back(parseLocation());
				} else if (match(ERROR_PAGE)) {
					std::map<int, std::string> tempMap = parseErrorPage();
					tempConfig.common.errorPage.insert(tempMap.begin(), tempMap.end());
				} else if (match(RETURN)) {
					parseReturn(tempConfig.common.returnPath, &tempConfig.common.returnCode);
				} else if (match(CGI)) {
					std::string extension;
					consume(DOT, "expected an extension type with a dot, example: .py");
					extension = "." + consume(IDENTIFIER, "expected an extension type with a dot, example: .py").getAll();
					tempConfig.common.cgi.insert(std::pair<std::string, std::string>(extension, parsePath()));
					consume(SEMICOLON, "expected ';' after the statement");
				} else if (match(MAX_CLIENT_BODY_SIZE)) {
					tempConfig.common.max_client_body = parseMaxClientBody();
					consume(SEMICOLON, "expected ';' after the statement");
				} else {
					throw std::runtime_error("Unexpected keyword found: " + tokenPeek().getAll());
				}
			}
			consume(RIGHT_BRACE, "expected terminator '}' after server block content");
			_configVector.push_back(tempConfig);
		} else {
			throw std::runtime_error("expected server block, got \"" + tokenPeek().getAll() + "\" instead");
		}
	}

	split_host_and_address();
}