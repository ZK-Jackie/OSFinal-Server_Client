cmake_minimum_required(VERSION 3.26)
project(server C)

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(CMAKE_C_STANDARD 11)

find_package(Threads REQUIRED)

add_executable(server main.c
        utils/param.c
        utils/param.h
        OSServerApplication.c
        utils/log.c
        utils/log.h
        thread/thread.c
        thread/thread.h
        common/constants.h
        socket/isocket.c
        socket/isocket.h)

target_link_libraries(server PRIVATE Threads::Threads)