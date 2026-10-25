add_library(file_ptr INTERFACE file_ptr.hpp)
target_include_directories(file_ptr INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(file_ptr_test file_ptr_test.cpp)
target_link_libraries(file_ptr_test PRIVATE file_ptr unique_fd GTest::gtest_main project_warnings)

include(GoogleTest)
gtest_discover_tests(file_ptr_test)
