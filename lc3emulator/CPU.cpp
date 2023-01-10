#include "CPU.hpp"

#include <fstream>
#include <iostream>

bool CPU::load(const std::string& fileToRun) 
{
    std::ifstream ifs(fileToRun, std::ios::binary);
    if (!ifs.is_open()) {
        return false;
    }

    uint16_t origin;
    ifs.read(reinterpret_cast<char*>(&origin), sizeof(origin));
    m_pc = origin;

    while (!ifs.eof()) {
        uint16_t data;
        ifs.read(reinterpret_cast<char*>(&data), sizeof(data));
        m_memory[origin++] = data;
    } 
    dumpMemory(m_pc, 5);
    return true;
}

bool CPU::emulate()
{
    return true;
}

void CPU::dumpMemory(uint16_t start, uint16_t size)
{
    for (uint16_t i = start; i < start + size; ++i) {
        std::cout << "memory[ " << i << " ]" << " = " << m_memory[i] << std::endl; 
    }
}
