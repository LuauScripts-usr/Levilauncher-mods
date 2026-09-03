cmake_minimum_required(VERSION 3.18.1)
project(ControllerOverdrive)

set(CMAKE_CXX_STANDARD 17)
# 10000x Performance Flags: O3, fast-math, LTO, and stripping RTTI/Exceptions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -fno-rtti -fno-exceptions -flto")

add_library(ControllerOverdrive SHARED src/ControllerOverdrive.cpp)
target_link_libraries(ControllerOverdrive log dl)
