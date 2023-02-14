#include <exception>
#include <iostream>

#include "CPU.hpp"

int main(int argc, char* argv[])
{
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    if (argc < 2) {
        std::cout << "usage: lc3emulator filename" << std::endl;
        return -1;
    }

    std::string fileToRun = argv[1];
    try {
        CPU cpu;
        cpu.load(fileToRun);
        cpu.emulate();
    }
    catch (std::exception e) {
        std::cout << "LC3 EMULATOR ERROR: " << e.what() << std::endl;
    }
}