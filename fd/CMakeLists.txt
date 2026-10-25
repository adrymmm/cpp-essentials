add_library(unique_fd unique_fd.cpp)
target_include_directories(unique_fd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(unique_fd PRIVATE project_warnings)

add_executable(unique_fd_test unique_fd_test.cpp)
target_link_libraries(unique_fd_test PRIVATE unique_fd GTest::gtest_main project_warnings)

include(GoogleTest)
gtest_discover_tests(unique_fd_test)
