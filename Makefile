NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -g
SRC = main.cpp Parser/Parser.cpp \
				Parser/Token.cpp
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