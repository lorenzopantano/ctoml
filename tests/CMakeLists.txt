add_executable(test_ctoml test_ctoml.c)
target_link_libraries(test_ctoml ctoml)

# Propagate ASan to test executable (if library is sanitized)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(test_ctoml PRIVATE -O1 -g -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(test_ctoml PRIVATE -fsanitize=address)
endif()

add_test(NAME basic_first_test COMMAND test_ctoml)