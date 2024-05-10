cmake_minimum_required(VERSION 3.26)
project(server C)

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(CMAKE_C_STANDARD 11)

add_executable(server main.c)