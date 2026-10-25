cmake_minimum_required(VERSION 3.24)
project(cpp_essentials LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)          # -std=c++20, not gnu++20
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # for clangd

# Sanitizers on everything (incl. GoogleTest) so the whole binary is instrumented
add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
add_link_options(-fsanitize=address,undefined)

include(FetchContent)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0)
FetchContent_MakeAvailable(googletest)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic -Wshadow)

enable_testing()
add_subdirectory(fd)
add_subdirectory(buffer)
add_subdirectory(counting)
add_subdirectory(cfile)
