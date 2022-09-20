cmake_minimum_required(VERSION 3.22)
project(lambda-calculus)

set(CMAKE_CXX_STANDARD 14)

add_executable(lci src/main.cpp)

add_executable(lci_tests test/test.cpp)

add_executable(build tools/build.cpp)

include_directories(.)
