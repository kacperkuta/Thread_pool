cmake_minimum_required(VERSION 3.13)
project(Thread_pool C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(Thread_pool main.c threadpool.h err.h err.c)