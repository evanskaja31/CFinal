cmake_minimum_required(VERSION 3.14)
project(CFinal C)

set(CMAKE_C_STANDARD 99)

add_executable(CFinal main.c)

target_link_libraries(CFinal m)