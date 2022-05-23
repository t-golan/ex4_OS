cmake_minimum_required(VERSION 3.22)
project(ex4_OS)

set(CMAKE_CXX_STANDARD 11)

add_executable(ex4_OS PhysicalMemory.h PhysicalMemory.cpp VirtualMemory.h VirtualMemory.cpp SimpleTest.cpp)
