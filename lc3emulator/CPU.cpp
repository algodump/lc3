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

Register getDestinationRegisterNumber(uint16_t insturction)
{
    return static_cast<Register>(retrieveBits(insturction, 11, 3));
}

Register getSourceBaseRegisterNumber(uint16_t insturction)
{
    return static_cast<Register>(retrieveBits(insturction, 8, 3));
}
} // namespace

CPU::CPU() : m_conditionalCodes{false, false, false} {}

InstructionOpCode CPU::getOpCode(uint16_t instruction) const
{
    return static_cast<InstructionOpCode>(retrieveBits(instruction, 15, 4));
}

void CPU::setConditionalCodes(Register destinationRegisterNumber)
{
    auto destinationRegisterNumberValue = m_registers[destinationRegisterNumber];
    if (destinationRegisterNumberValue < 0) {
        m_conditionalCodes.N = true;
    } else if (destinationRegisterNumberValue == 0) {
        m_conditionalCodes.Z = true;
    } else {
        m_conditionalCodes.P = true;
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
    dumpMemory(m_pc, 5);
    return true;
}

bool CPU::emulate(uint16_t instruction) 
{
    auto opCode = getOpCode(instruction);
    switch (opCode) {
    case InstructionOpCode::ADD: {
        Register destinationRegisterNumber = getDestinationRegisterNumber(instruction);
        Register sourceRegisterNumber = getSourceBaseRegisterNumber(instruction);
        if ((instruction >> 5) & 0x1) {
            uint8_t immediateValue = retrieveBits(instruction, 4, 5);
            m_registers[destinationRegisterNumber] =
                m_registers[sourceRegisterNumber] + immediateValue;
        }
        else {
            Register secondSource = static_cast<Register>(retrieveBits(instruction, 2, 3));
            m_registers[destinationRegisterNumber] =
                m_registers[sourceRegisterNumber] + m_registers[secondSource];
        }
        setConditionalCodes(sourceRegisterNumber);
        break;
    }
    case InstructionOpCode::AND: {
        Register destinationRegisterNumber = getDestinationRegisterNumber(instruction);
        Register sourceRegisterNumber = getSourceBaseRegisterNumber(instruction);
        if ((instruction >> 5) & 0x1) {
            uint8_t immediateValue = retrieveBits(instruction, 4, 5);
            m_registers[destinationRegisterNumber] =
                m_registers[sourceRegisterNumber] & immediateValue;
        }
        else {
            Register secondSourceReigsterNumber =
                static_cast<Register>(retrieveBits(instruction, 2, 3));
            m_registers[destinationRegisterNumber] =
                m_registers[sourceRegisterNumber] & m_registers[secondSourceReigsterNumber];
        }
        setConditionalCodes(destinationRegisterNumber);
        break;
    }
    case InstructionOpCode::BR: {
        uint8_t n = (instruction >> 11) & 0x1;
        uint8_t z = (instruction >> 10) & 0x1;
        uint8_t p = (instruction >> 9) & 0x1;

        int16_t offset = retrieveBits(instruction, 8, 9);

        if ((n && m_conditionalCodes.N) || (z && m_conditionalCodes.Z) ||
            (p && m_conditionalCodes.P)) {
                m_pc += offset;
        } else if (n == 0 && z == 0 && p == 0) {
            m_pc += offset;
        }
        break;
    }
    case InstructionOpCode::JMP_RET: {
        Register baseRegisterNumber = getSourceBaseRegisterNumber(instruction);
        m_pc = m_registers[baseRegisterNumber];
        break;
    }
    case InstructionOpCode::JSR_JSRR: {
        m_registers[R7] = m_pc;
        // is JSR
        if ((instruction >> 11) & 0x1) {
            int16_t offset = retrieveBits(instruction, 10, 11);
            m_pc += offset;
        } else {
            Register baseRegisterNumber = getSourceBaseRegisterNumber(instruction);
            m_pc = m_registers[baseRegisterNumber];
        }
        break;
    }
    case InstructionOpCode::LD: {
        Register destinationRegisterNumber = getDestinationRegisterNumber(instruction);
        int16_t offset = retrieveBits(instruction, 8, 9);
        m_registers[destinationRegisterNumber] = m_memory[m_pc + offset];
        setConditionalCodes(destinationRegisterNumber);
        break;
    }
    case InstructionOpCode::LDI: {
        Register destinationRegisterNumber = getDestinationRegisterNumber(instruction);
        int16_t offset = retrieveBits(instruction, 8, 9);

        m_registers[destinationRegisterNumber] =
            m_memory[m_memory[m_pc + offset]];
        setConditionalCodes(destinationRegisterNumber);
        break;
    }
    case InstructionOpCode::LDR: {
        Register destinationRegisterNumber =
            getDestinationRegisterNumber(instruction);
        Register baseRegisterNumber = getSourceBaseRegisterNumber(instruction);
        int16_t immediateValue = retrieveBits(instruction, 5, 6);

        m_registers[destinationRegisterNumber] =
            m_registers[baseRegisterNumber] + immediateValue;
        setConditionalCodes(destinationRegisterNumber);
        break;
    }
    case InstructionOpCode::LEA: {
        Register destinationRegisterNumber =
            getDestinationRegisterNumber(instruction);
        int16_t labelOffset = retrieveBits(instruction, 8, 9);
        m_registers[destinationRegisterNumber] = m_pc + labelOffset;
        setConditionalCodes(destinationRegisterNumber);
        break;
    }
    case InstructionOpCode::NOT: {
        Register destinationRegisterNumber =
            getDestinationRegisterNumber(instruction);
        Register sourceRegisterNumber = getSourceBaseRegisterNumber(instruction);
        m_registers[destinationRegisterNumber] = ~(m_registers[sourceRegisterNumber]);
        setConditionalCodes(destinationRegisterNumber);
        break;
    }
    case InstructionOpCode::RTI: {
        break;
    }
    case InstructionOpCode::ST: {
        Register sourceRegisterNumber =
            getDestinationRegisterNumber(instruction);
        int16_t labelOffset = retrieveBits(instruction, 8, 9);
        m_memory[m_pc + labelOffset] = m_registers[sourceRegisterNumber];
        break;
    }
    case InstructionOpCode::STI: {
        Register sourceRegisterNumber =
            getDestinationRegisterNumber(instruction);
        int16_t labelOffset = retrieveBits(instruction, 8, 9);
        m_memory[m_memory[m_pc + labelOffset]] =
            m_registers[sourceRegisterNumber];
        break;
    }
    case InstructionOpCode::STR: {
        Register sourceRegisterNumber =
            getDestinationRegisterNumber(instruction);
        Register baseRegisterNumber =
            getSourceBaseRegisterNumber(instruction);
        int16_t labelOffset = retrieveBits(instruction, 5, 6);
        m_memory[m_registers[baseRegisterNumber] + labelOffset] =
            m_registers[sourceRegisterNumber];
        break;
    }
    case InstructionOpCode::TRAP: {
        break;
    }
    default: {
        assert(false && "illegal instruction");
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
        if (getOpCode(instruction) == InstructionOpCode::JMP_RET &&
            getSourceBaseRegisterNumber(instruction) == R7) {
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