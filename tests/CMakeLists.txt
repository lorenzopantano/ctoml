add_executable(test_ctoml test_ctoml.c)
target_link_libraries(test_ctoml ctoml)
add_test(NAME basic_first_test COMMAND test_ctoml)