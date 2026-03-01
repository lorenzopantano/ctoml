cmake_minimum_required(VERSION 3.23)
project(ctoml)

add_library(ctoml src/file.c src/lexer.c src/stream.c src/parser.c)
target_include_directories(ctoml PUBLIC include)

# AddressSanitizer for Debug builds (macOS/Clang)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(ctoml PRIVATE -O1 -g -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(ctoml PRIVATE -fsanitize=address)
endif()

enable_testing()
find_program(VALGRIND_EXECUTABLE valgrind)
if(VALGRIND_EXECUTABLE)
  message(STATUS "Valgrind found: ${VALGRIND_EXECUTABLE}")
  set(MEMORYCHECK_COMMAND ${VALGRIND_EXECUTABLE})
  set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --show-leak-kinds=all --error-exitcode=1")
endif()

add_subdirectory(tests)