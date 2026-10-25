add_library(buffer buffer.cpp)
target_include_directories(buffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(buffer PRIVATE project_warnings)

add_executable(buffer_test buffer_test.cpp)
target_link_libraries(buffer_test PRIVATE buffer GTest::gtest_main project_warnings)

include(GoogleTest)
gtest_discover_tests(buffer_test)
