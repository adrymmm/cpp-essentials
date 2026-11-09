add_executable(spsc_bench spsc_bench.cpp)
target_link_libraries(spsc_bench PRIVATE spsc spsc_atomic project_warnings) 

