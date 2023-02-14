cmake_minimum_required(VERSION 3.12)
project(lc3asm VERSION 0.1.0)

include_directories(../fmt/include)
add_executable(lc3asm main.cpp reader.cpp instructions.cpp assembler.cpp)
target_link_libraries(lc3asm PRIVATE fmt)

add_subdirectory(tests)