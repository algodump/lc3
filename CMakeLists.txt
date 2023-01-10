cmake_minimum_required(VERSION 3.12)
project(lc3 VERSION 0.1.0)

set(gtest_force_shared_crt on)
set(CMAKE_CXX_FLAGS "/std:c++latest /EHsc")

add_subdirectory(lc3assembler)
add_subdirectory(lc3emulator)
add_subdirectory(googletest)