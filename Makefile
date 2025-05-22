NAME = ./sockets
CXX = c++ -std=c++98
CXXFLAGS = -Wall -Werror -Wextra -g

SRC = Sockets/ASocket.cpp \
	Sockets/ClientSocket.cpp \
	Sockets/ServerSocket.cpp \
	Sockets/ListenerSocket.cpp \
	main.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: clean all

.PHONY: all clean fclean re