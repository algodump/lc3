#include "CPU.hpp"

#include <assert.h>
#include <bitset>
#include <fstream>
#include <iostream>


namespace {
int16_t retrieveBits(uint16_t insturction, uint8_t start, uint8_t size)
{
    assert(start <= 15 && start >= 0);
    uint16_t mask = (1 << size) - 1;
    int16_t res = (insturction >> (start - size + 1)) & mask;
    return res;
}

uint16_t getDestinationRegister(uint16_t insturction)
{
    return retrieveBits(insturction, 11, 3);
}

uint16_t getSourceBaseRegister(uint16_t insturction)
{
    return retrieveBits(insturction, 8, 3);
}
} // namespace

CPU::CPU() : m_conditionalCodes{0, 0, 0} {}

InstructionOpCode CPU::getOpCode(uint16_t instruction) const
{
    return static_cast<InstructionOpCode>(retrieveBits(instruction, 15, 4));
}

void CPU::setconDitionalCodes(uint8_t destinationRegister)
{
    auto destinationRegisterValue = m_memory[destinationRegister];
    if (destinationRegisterValue < 0) {
        m_conditionalCodes.N = 1;
    } else if (destinationRegisterValue == 0) {
        m_conditionalCodes.Z = 1;
    } else {
        m_conditionalCodes.P = 1;
    }
}

bool CPU::load(const std::string& fileToRun)
{
    std::ifstream ifs(fileToRun, std::ios::binary);
    if (!ifs.is_open()) {
        return false;
    }
    uint16_t origin;
    ifs.read(reinterpret_cast<char*>(&origin), sizeof origin);
    m_pc = origin;

    while (!ifs.eof()) {
        uint16_t data;
        ifs.read(reinterpret_cast<char*>(&data), sizeof(data));
        if (!ifs.fail()) {
            m_memory[origin++] = data;
        }
    }
    // TODO: find out what are the terminating conditions
    m_memory[origin] = 0xDEAD;
    dumpMemory(m_pc, 5);
    return true;
}

bool CPU::emulate(uint16_t instruction) 
{
    auto opCode = getOpCode(instruction);
    switch (opCode) {
    case InstructionOpCode::ADD: {
        uint8_t destinationRegister = getDestinationRegister(instruction);
        uint8_t sourceRegister = getSourceBaseRegister(instruction);
        if ((instruction >> 5) & 0x1) {
            uint8_t immediateValue = retrieveBits(instruction, 4, 5);
            m_registers[destinationRegister] =
                m_registers[sourceRegister] + immediateValue;
        }
        else {
            uint8_t secondSource = retrieveBits(instruction, 2, 3);
            m_registers[destinationRegister] =
                m_registers[sourceRegister] + m_registers[secondSource];
        }
        setconDitionalCodes(destinationRegister);
        break;
    }
    case InstructionOpCode::AND: {
        uint8_t destinationRegister = getDestinationRegister(instruction);
        uint8_t sourceRegister = getSourceBaseRegister(instruction);
        if ((instruction >> 5) & 0x1) {
            uint8_t immediateValue = retrieveBits(instruction, 4, 5);
            m_registers[destinationRegister] =
                m_registers[sourceRegister] & immediateValue;
        }
        else {
            uint8_t secondSource = retrieveBits(instruction, 2, 3);
            m_registers[destinationRegister] =
                m_registers[sourceRegister] & m_registers[secondSource];
        }
        setconDitionalCodes(destinationRegister);
        break;
    }
    case InstructionOpCode::BR: {
        uint8_t n = retrieveBits(instruction, 11, 1);
        uint8_t z = retrieveBits(instruction, 10, 1);
        uint8_t p = retrieveBits(instruction, 9, 1);

        int16_t offset = retrieveBits(instruction, 8, 9);

        if ((n && m_conditionalCodes.N) || (z && m_conditionalCodes.Z) ||
            (p && m_conditionalCodes.P)) {
                m_pc += offset;
        } else if (n == 0 && z == 0 && p == 0) {
            m_pc += offset;
        }
        break;
    }
    case InstructionOpCode::LD: {
        uint8_t destinationRegister = getDestinationRegister(instruction);
        int16_t offset = retrieveBits(instruction, 8, 9);
        m_registers[destinationRegister] = m_memory[m_pc + offset];
        setconDitionalCodes(destinationRegister);
        break;
    }
    }
    return true;
}

bool CPU::emulate()
{
    while (true) {
        // User is responsible for not mixing data and insturctions
        // as emulator can't differentiate insturction from
        // raw data.
        uint16_t instruction = m_memory[m_pc++];
        if (instruction == 0xDEAD) {
            break;
        }
        emulate(instruction);
    }
    return true;
}

void CPU::dumpMemory(uint16_t start, uint16_t size)
{
    for (uint16_t i = start; i < start + size; ++i) {
        std::cout << "memory[ " << i << " ]"
                  << " = " << m_memory[i] << std::endl;
    }
}