NAME = ./webserv
INCLUDE_DIR = -I Sockets -I Server -I Parser
CXX = c++ -std=c++98
CXXFLAGS = -Wall -Werror -Wextra -g $(INCLUDE_DIR)

SRC = Sockets/ASocket.cpp \
	Sockets/ClientSocket.cpp \
	Sockets/ServerSocket.cpp \
	Server/IServer.cpp \
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

.PHONY: all clean fclean re