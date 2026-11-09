add_library(spsc INTERFACE spsc_queue.hpp)
target_include_directories(spsc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(spsc INTERFACE ring Threads::Threads)

add_library(spsc_atomic INTERFACE spsc_queue_atomic.hpp)
target_include_directories(spsc_atomic INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(spsc_atomic INTERFACE Threads::Threads)

add_executable(spsc_queue_test spsc_queue_test.cpp)
target_link_libraries(spsc_queue_test PRIVATE spsc spsc_atomic GTest::gtest_main project_warnings)

include(GoogleTest)
gtest_discover_tests(spsc_queue_test PROPERTIES TIMEOUT 10)
