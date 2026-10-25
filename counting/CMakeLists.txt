add_executable(counting_test counting_test.cpp)
target_link_libraries(counting_test PRIVATE GTest::gtest_main project_warnings)

include(GoogleTest)
gtest_discover_tests(counting_test)
