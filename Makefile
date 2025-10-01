NAME = ./webserv
INCLUDE_DIR = -I Sockets -I Server -I Parser -I Utilities -I Logger
CXX = c++ -std=c++98 -g -static-libasan
CXXFLAGS = -Wall -Wextra -Werror $(INCLUDE_DIR)
LOGSDIR = Logs
SCRIPTS = mkdir -p $(LOGSDIR)

SRC = \
	Server/ServerSocketManager.cpp \
	Server/ServerSocket.cpp \
	Server/ClientSocket.cpp \
	Server/Socket.cpp \
	Server/Server.cpp \
	Server/ServerResponse.cpp \
	Server/ServerEvent.cpp \
	Logger/ServerLogger.cpp \
	Logger/ErrorLogger.cpp \
	Logger/AccessLogger.cpp \
	Logger/ALogger.cpp \
	Utilities/fs.cpp \
	Utilities/status.cpp \
	Parser/Parser.cpp \
	Parser/Token.cpp \
	main.cpp

OBJ = $(SRC:.cpp=.o)
PARSER_TEST_EXEC = parser_test
PARSER_TEST_SRCS = Tests/parser/parser_test.cpp Parser/Parser.cpp Parser/Token.cpp
PARSER_TEST_FLAGS = -std=c++14 -Wall -Werror -Wextra -fsanitize=address -lgtest -lgtest_main -lpthread

all: $(NAME) scripts

scripts:
	$(SCRIPTS)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(INCLUDE) -o $(NAME)

clean:
	rm -rf $(OBJ) 

fclean: clean
	rm -rf $(NAME) $(LOGSDIR) $(PARSER_TEST_EXEC)

re: fclean all

docker_build:
	docker compose up --build --yes -d

run: docker_build
	docker attach webserv

$(PARSER_TEST_EXEC): $(PARSER_TEST_SRCS)
	$(CXX) $(PARSER_TEST_SRCS) $(PARSER_TEST_FLAGS) -o $(PARSER_TEST_EXEC)

test_parser: $(PARSER_TEST_EXEC)
	./$(PARSER_TEST_EXEC)

.PHONY: all clean fclean re run docker_build
