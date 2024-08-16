cmake_minimum_required(VERSION 3.26)
project(babymalloc C)

set(CMAKE_C_STANDARD 17)

add_library(babymalloc babymalloc.c)
add_executable(test test.c)
target_link_libraries(test babymalloc)
