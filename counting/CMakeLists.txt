add_executable(counting_test counting_test.cpp)
# Warnings on our code only; GoogleTest's code would flood -Wshadow etc.
target_compile_options(counting_test PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
target_link_libraries(counting_test PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(counting_test)
