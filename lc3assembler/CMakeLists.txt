cmake_minimum_required(VERSION 3.12)
project(lc3asm VERSION 0.1.0)

add_executable(lc3asm main.cpp reader.cpp instructions.cpp assembler.cpp)
add_subdirectory(tests)

include(CTest)
add_test(AssemblerTest ..\\build\\tests\\assemblerTest.exe)
add_test(ReaderTest ..\\build\\tests\\readerTest.exe)