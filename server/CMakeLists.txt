cmake_minimum_required(VERSION 3.26)
project(server C)

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(CMAKE_C_STANDARD 11)

# 添加pthread链接库(1/2)
find_package(Threads REQUIRED)

# 添加头文件搜索路径
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(../common/include)
# 自动搜索、链接src目录下的所有.c .h文件
file(GLOB_RECURSE SOURCES "src/*.c" "include/*.h")
# 添加common目录下的所有.c文件
file(GLOB_RECURSE COMMON_SOURCES "../common/src/*.c")

add_executable(server  ${SOURCES} ${COMMON_SOURCES})

# 添加pthread链接库(2/2)
target_link_libraries(server PRIVATE Threads::Threads)