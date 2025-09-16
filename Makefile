NAME = ./webserv
INCLUDE_DIR = -I Sockets -I Server -I Parser -I Utilities -I Logger
CXX = c++ -std=c++98 -g -static-libasan
CXXFLAGS = -Wall -Wextra -Werror $(INCLUDE_DIR)

BUILDDIR = Build
OBJSDIR = $(BUILDDIR)/Objs
EXECUTABLEDIR = $(BUILDDIR)/

LIBDIR = $(BUILDDIR)/Lib
LIB = webservlib.a

LOGSDIR = Logs
SCRIPTS = mkdir -p $(LOGSDIR)

UNIT_TESTS_DIR = Tests/unit
TESTS_DIR = Tests/global
VENV_DIR = $(TESTS_DIR)/venv

TESTS_SRCS = \
	Tests/test.py

SRCS = \
	Server/ServerRequestParser.cpp \
	Server/ServerRequestParserHelpers.cpp \
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

OBJS = $(patsubst %.cpp, $(OBJSDIR)/%.o, $(SRCS))

all: $(NAME) scripts

$(OBJS): $(OBJSDIR)/%.o: %.cpp
	mkdir -p $(BUILDDIR) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< $(INCLUDE) -o $@

$(LIBDIR)/$(LIB): $(OBJS)
	mkdir -p $(LIBDIR)
	ar rcs $(LIBDIR)/$(LIB) $(OBJS)

$(OBJSDIR)/main.o: main.cpp
	mkdir -p $(OBJSDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(NAME): $(OBJSDIR)/main.o $(LIBDIR)/$(LIB)
	$(CXX) $(OBJSDIR)/main.o $(LIBDIR)/$(LIB) -o $@

scripts:
	$(SCRIPTS)

clean:
	rm -rf $(OBJSDIR)/$(dir $(OBJS))

fclean:
	rm -rf $(BUILDDIR) $(LOGSDIR) $(NAME)

re: fclean all

docker_build:
	docker compose up --build --yes -d

run: docker_build
	docker attach webserv

# install_python_libs:
# 	python3 -m venv $(VENV_DIR)
# 	./$(VENV_DIR)/bin/pip install --upgrade pip
# 	./$(VENV_DIR)/bin/pip install -r $(TESTS_DIR)/requirements.txt

# prepare_test_env: all install_python_libs

# test: prepare_test_env
# 	. $(VENV_DIR)/bin/activate; python3 ./$(TESTS_DIR)/test.py

cmake_configure:
	cmake -DCMAKE_BUILD_TYPE:STRING=Debug --no-warn-unused-cli -S $(UNIT_TESTS_DIR) -B $(BUILDDIR) -G "Unix Makefiles"

test: all cmake_configure
	cmake --build $(BUILDDIR) --config Debug --target all -j8	
	ctest --test-dir $(BUILDDIR)

.PHONY: all clean fclean re run docker_build test install_python_libs prepare_test_env