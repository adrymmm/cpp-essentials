add_library(ring_buffer INTERFACE ring_buffer.hpp)
target_include_directories(ring_buffer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ring_buffer_test ring_buffer_test.cpp)
target_link_libraries(ring_buffer_test PRIVATE ring_buffer GTest::gtest_main project_warnings)

include(GoogleTest)
gtest_discover_tests(ring_buffer_test)
