cmake_minimum_required(VERSION 3.12)
project(lc3emulator VERSION 0.1.0)

include_directories(../fmt/include)
add_executable(lc3emulator main.cpp CPU.cpp)
target_link_libraries(lc3emulator PRIVATE fmt)

add_subdirectory(emulatorTests)