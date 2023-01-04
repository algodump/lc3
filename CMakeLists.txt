cmake_minimum_required(VERSION 3.0.0)
project(lc3asm VERSION 0.1.0)


set (CMAKE_CXX_FLAGS "/std:c++latest")

add_executable(lc3asm main.cpp reader.cpp instructions.cpp assembler.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
