cmake_minimum_required(VERSION 3.17)
project(server C)

set(CMAKE_C_STANDARD 99)

add_executable(server main.c cJSON.c cJSON.h)
target_link_libraries(server ws2_32)