NAME = Webserv
CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -g
SRC = main.cpp Parser.cpp Token.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all fclean re clean