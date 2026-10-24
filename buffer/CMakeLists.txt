add_library(buffer buffer.cpp)
target_include_directories(buffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(buffer PRIVATE -Wall -Wextra -Wpedantic -Wshadow)

add_executable(buffer_test buffer_test.cpp)
# Warnings on our code only; GoogleTest's code would flood -Wshadow etc.
target_compile_options(buffer_test PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
target_link_libraries(buffer_test PRIVATE buffer GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(buffer_test)
