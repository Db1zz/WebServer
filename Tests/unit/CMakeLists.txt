cmake_minimum_required(VERSION 3.14)
project(tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED 17)

include_directories(../../)
include_directories(../../Server)
include_directories(../../Server/RequestParser)
include_directories(../../Logger)
include_directories(../../Parser)
include_directories(../../Utilities)

set(SOURCES
  ServerRequestParserTests.cpp
  RequestMultipartBodyParserTests.cpp
  ServerRequestParserHelpersTests.cpp
  RequestHeaderParserTests.cpp
  ServerResponseTests.cpp
)

include(ExternalProject)

set(WEBSERVER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
set(WEBSERVER_LIB_PATH ${WEBSERVER_DIR}/Build/lib/webservlib.a)

ExternalProject_Add(webserver_project
  SOURCE_DIR ${WEBSERVER_DIR}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make -C ${WEBSERVER_DIR}
  INSTALL_COMMAND ""
  BUILD_BYPRODUCTS ${WEBSERVER_LIB_PATH}
)

add_library(webserver_lib STATIC IMPORTED)

set_target_properties(webserver_lib PROPERTIES
  IMPORTED_LOCATION ${WEBSERVER_LIB_PATH}
  INTERFACE_INCLUDE_DIRECTORIES ${WEBSERVER_DIR}
)

add_dependencies(webserver_lib webserver_project)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  tests ${SOURCES}
)

target_link_libraries(
  tests
  PUBLIC
  webserver_lib
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
