#include <iostream>

#include "CPU.hpp"

int main()
{
    std::string file_to_run = "C:\\Users\\name\\Desktop\\Code\\lc3\\out.l3_bin";
    CPU cpu;
    if (cpu.load(file_to_run)) {
        cpu.emulate();
    }
    else {
        std::cout << "ERROR: Couldn't load file: " << file_to_run << std::endl;
    }
}