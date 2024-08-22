cmake_minimum_required(VERSION 3.26)
project(babymalloc C)

set(CMAKE_C_STANDARD 17)

# Uncomment the following lines to compile for 32-bit (used for testing)
#set(CMAKE_C_FLAGS -m32)
#set(CMAKE_CXX_FLAGS -m32)

add_library(babymalloc babymalloc.c)
add_executable(test test.c)
target_link_libraries(test babymalloc)
