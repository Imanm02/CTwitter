cmake_minimum_required(VERSION 3.17)
project(main C)

set(CMAKE_C_STANDARD 99)

add_executable(main main.c cJSON.c cJSON.h)
target_link_libraries(main ws2_32)
