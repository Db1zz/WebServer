NAME = ./webserv
INCLUDE_DIR = -I Sockets -I Server -I Parser -I Utilities -I Logger
CXX = c++ -std=c++98 -g -static-libasan
CXXFLAGS = -Wall -Wextra -Werror $(INCLUDE_DIR)
LOGSDIR = Logs
SCRIPTS = mkdir -p $(LOGSDIR)

TESTS_DIR = Tests
VENV_DIR = $(TESTS_DIR)/venv

TESTS_SRC = \
	Tests/test.py

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

all: $(NAME) scripts

scripts:
	$(SCRIPTS)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(INCLUDE) -o $(NAME)

clean:
	rm -rf $(OBJ) 

fclean: clean
	rm -rf $(NAME) $(LOGSDIR)

re: fclean all

docker_build:
	docker compose up --build --yes -d

run: docker_build
	docker attach webserv

test:
	. $(VENV_DIR)/bin/activate; python3 ./$(TESTS_DIR)/test.py

install_python_libs:
	python3 -m venv $(VENV_DIR)
	./$(VENV_DIR)/bin/pip install --upgrade pip
	./$(VENV_DIR)/bin/pip install -r $(TESTS_DIR)/requirements.txt

compile_and_test: re install_python_libs test

.PHONY: all clean fclean re run docker_build test install_python_libs compile_and_test