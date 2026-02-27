cmake_minimum_required(VERSION 3.23)
project(ctoml)

add_library(ctoml src/file.c src/lexer.c)
target_include_directories(ctoml PUBLIC include)

enable_testing()
add_subdirectory(tests)