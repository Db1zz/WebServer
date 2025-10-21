NAME = ./webserv
INCLUDE_DIR = -I Sockets -I Server -I Response -I Parser -I Utilities -I Logger
CXX = c++ -std=c++98 -g -static-libasan
CXXFLAGS = -Wall -Wextra -Werror $(INCLUDE_DIR)

BUILDDIR = Build
OBJSDIR = $(BUILDDIR)/Objs
EXECUTABLEDIR = $(BUILDDIR)/

LIBDIR = $(BUILDDIR)/Lib
LIB = webservlib.a

LOGSDIR = Logs

UNIT_TESTS_DIR = Tests/unit
TESTS_DIR = Tests/global
VENV_DIR = $(TESTS_DIR)/venv

TEMP_FILES_DIR = .tempfiles

PROJECT_DIRS = $(LOGSDIR) $(LIBDIR) $(TEMP_FILES_DIR)

TESTS_SRCS = \
	Tests/test.py

SRCS = \
	Server/RequestParser/ServerRequestParser.cpp \
	Server/RequestParser/ServerRequestParserHelpers.cpp \
	Server/RequestParser/RequestMultipartBodyParser.cpp \
	Server/RequestParser/RequestRawBodyParser.cpp \
	Server/RequestParser/RequestHeaderParser.cpp \
	Server/ServerSocketManager.cpp \
	Server/CGIFileDescriptor.cpp \
	Server/CGIResponseParser.cpp \
	Server/ServerUtils.cpp \
	Server/ServerSocket.cpp \
	Server/ClientSocket.cpp \
	Server/Socket.cpp \
	Server/Server.cpp \
	Server/ServerEvent.cpp \
	Response/ServerResponse.cpp \
	Response/JsonResponse.cpp \
	Response/ErrorResponse.cpp \
	Response/FileUtils.cpp \
	Response/Chunk.cpp \
	Logger/ServerLogger.cpp \
	Logger/ErrorLogger.cpp \
	Logger/AccessLogger.cpp \
	Logger/ALogger.cpp \
	Utilities/fs.cpp \
	Utilities/status.cpp \
	Utilities/hash.cpp \
	Utilities/timer.cpp \
	Parser/Parser.cpp \
	Parser/Token.cpp \

OBJS = $(patsubst %.cpp, $(OBJSDIR)/%.o, $(SRCS))

all: create_project_dirs $(NAME)

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

create_project_dirs:
	mkdir -p $(PROJECT_DIRS)

clean:
	rm -rf $(OBJSDIR)/$(dir $(OBJS))

fclean:
	rm -rf $(BUILDDIR) $(PROJECT_DIRS) $(NAME)

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

.PHONY: all clean fclean re run docker_build test install_python_libs prepare_test_env cmake_configure