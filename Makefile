NAME = ./webserv
INCLUDE_DIR = -I Sockets -I Server -I Parser -I Utilities -I Logger
CXX = c++ -std=c++98
CXXFLAGS = -Wall -Wextra $(INCLUDE_DIR)

SRC = \
	Server/ServerSocket.cpp \
	Server/Socket.cpp \
	Server/Server.cpp \
	Server/ServerResponse.cpp \
	Server/ServerEvent.cpp \
	Utilities/fs.cpp \
	Utilities/status.cpp \
	Parser/Parser.cpp \
	Parser/Token.cpp \
	main.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(INCLUDE) -o $(NAME)

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all

docker_build:
	docker compose up --build --yes -d

run: docker_build
	docker attach webserv

.PHONY: all clean fclean re run docker_build 