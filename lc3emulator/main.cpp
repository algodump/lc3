#include <iostream>

#include "CPU.hpp"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "usage: lc3emulator filename" << std::endl;
        return -1;
    }

    std::string fileToRun = argv[1];
    CPU cpu;
    if (cpu.load(fileToRun)) {
        cpu.emulate();
    }
    else {
        std::cout << "ERROR: Couldn't load file: " << fileToRun << std::endl;
    }
}