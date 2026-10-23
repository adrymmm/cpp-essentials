add_library(unique_fd unique_fd.cpp)
target_include_directories(unique_fd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(unique_fd PRIVATE -Wall -Wextra -Wpedantic -Wshadow)

add_executable(unique_fd_test unique_fd_test.cpp)
# Warnings on our code only; GoogleTest's code would flood -Wshadow etc.
target_compile_options(unique_fd_test PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
target_link_libraries(unique_fd_test PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(unique_fd_test)
