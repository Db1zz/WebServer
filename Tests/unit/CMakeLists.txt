cmake_minimum_required(VERSION 3.14)
project(tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED 17)

include_directories(../../)

set(SOURCES
  tests.cpp
  ServerRequestParserTests.cpp)

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
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
