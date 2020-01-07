cmake_minimum_required(VERSION 3.13)
project(Thread_pool C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(silnia threadpool.h threadpool.c err.h err.c future.h future.c silnia.c)
add_executable(macierz threadpool.h threadpool.c err.h err.c future.h future.c macierz.c)
add_executable(main threadpool.h threadpool.c err.h err.c future.h future.c main.c)