cmake_minimum_required(VERSION 3.12)

project(lc3asm)

add_subdirectory(../../googletest build)

include_directories(googletest/include)
list(APPEND testDependencies "../reader.cpp" "../instructions.cpp" "../assembler.cpp")
add_executable(readerTest readerTest.cpp ${testDependencies})
add_executable(assemblerTest assemblerTest.cpp ${testDependencies})

target_link_libraries(readerTest PRIVATE gtest)
target_link_libraries(assemblerTest PRIVATE gtest)