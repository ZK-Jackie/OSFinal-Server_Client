cmake_minimum_required(VERSION 3.26)
project(client C)

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(CMAKE_C_STANDARD 11)

# 添加pthread链接库(1/2)
find_package(Threads REQUIRED)

# 添加头文件搜索路径
include_directories(${CMAKE_SOURCE_DIR}/include)
# 自动搜索、链接src目录下的所有.c .h文件
file(GLOB_RECURSE SOURCES "src/*.c" "include/*.h")
add_executable(client  ${SOURCES})

# 添加pthread链接库(2/2)
target_link_libraries(client PRIVATE Threads::Threads)