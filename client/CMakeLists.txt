cmake_minimum_required(VERSION 3.26)
project(client C)

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(CMAKE_C_STANDARD 11)

add_executable(client main.c
        OSClientApplication.c
        utils/log.c
        utils/log.h
        utils/param.c
        utils/param.h
        socket/isocket.c
        socket/isocket.h)