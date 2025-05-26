NAME = ./webserv
INCLUDE_DIR = -I Sockets -I Server
CXX = c++ -std=c++98
CXXFLAGS = -Wall -Werror -Wextra -g $(INCLUDE_DIR)

SRC = Sockets/ASocket.cpp \
	Sockets/ClientSocket.cpp \
	Sockets/ServerSocket.cpp \
	Server/IServer.cpp \
	Server/Server.cpp \
	Server/ServerEvent.cpp \
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