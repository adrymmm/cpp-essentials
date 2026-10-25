add_library(file_ptr INTERFACE file_ptr.hpp)
target_include_directories(file_ptr INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(file_ptr_test file_ptr_test.cpp)
# Warnings on our code only; GoogleTest's code would flood -Wshadow etc.
target_compile_options(file_ptr_test PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
target_link_libraries(file_ptr_test PRIVATE file_ptr unique_fd GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(file_ptr_test)
