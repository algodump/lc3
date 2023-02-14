cmake_minimum_required(VERSION 3.12)

project(lc3emulator)
include_directories(googletest/include)
list(APPEND testDependencies "../CPU.cpp")
add_executable(emulatorTests emulatorTests.cpp ${testDependencies})

target_link_libraries(emulatorTests PRIVATE gtest fmt)