#include "CPU.hpp"

#include <assert.h>
#include <bitset>
#include <format>
#include <fstream>
#include <iostream>

namespace {
uint16_t retrieveBits(uint16_t insturction, uint8_t start, uint8_t size)
{
    assert(start <= 15 && start >= 0);
    uint16_t mask = (1 << size) - 1;
    uint16_t res = (insturction >> (start - size + 1)) & mask;
    return res;
}

uint16_t signExtend(uint16_t offset, uint8_t bitCount)
{
    if ((offset >> (bitCount - 1)) & 0x1) {
        offset |= (0xFFFF << bitCount);
    }
    return offset;
}

uint16_t signExtendRetriveBits(uint16_t insturction, uint8_t start,
                               uint8_t size)
{
    return signExtend(retrieveBits(insturction, start, size), size);
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

CPU::CPU() : m_registers{}, m_conditionalCodes{false, false, false} {}

InstructionOpCode CPU::getOpCode(uint16_t instruction) const
{
    return static_cast<InstructionOpCode>(retrieveBits(instruction, 15, 4));
}

void CPU::setConditionalCodes(Register destinationRegisterNumber)
{
    auto destinationRegisterNumberValue =
        m_registers[destinationRegisterNumber];
    if (destinationRegisterNumberValue >> 15) {
        m_conditionalCodes = {.N = true, .Z = false, .P = false};
    }
    else if (destinationRegisterNumberValue == 0) {
        m_conditionalCodes = {.N = false, .Z = true, .P = false};
    }
    else {
        m_conditionalCodes = {.N = false, .Z = false, .P = true};
    }
}

void CPU::load(const std::string& fileToRun)
{
    std::ifstream ifs(fileToRun, std::ios::binary);
    if (!ifs.is_open()) {
        throw std::runtime_error(
            std::format("Couldn't open a file: `{}`", fileToRun));
    }

    uint16_t origin;
    ifs.read(reinterpret_cast<char*>(&origin), sizeof origin);
    m_pc = origin;

    while (!ifs.eof()) {
        uint16_t data;
        ifs.read(reinterpret_cast<char*>(&data), sizeof data);
        if (!ifs.fail()) {
            m_memory.write(origin++, data);
        }
    }
    dumpMemory(m_pc, 5);
}

void CPU::emulate(uint16_t instruction)
{
    try {
        auto opCode = getOpCode(instruction);
        switch (opCode) {
        case InstructionOpCode::ADD: {
            Register destinationRegisterNumber =
                getDestinationRegisterNumber(instruction);
            Register sourceRegisterNumber =
                getSourceBaseRegisterNumber(instruction);
            if ((instruction >> 5) & 0x1) {
                uint16_t immediateValue = signExtendRetriveBits(instruction, 4, 5);
                m_registers[destinationRegisterNumber] =
                    m_registers[sourceRegisterNumber] + immediateValue;
            }
            else {
                Register secondSource =
                    static_cast<Register>(retrieveBits(instruction, 2, 3));
                m_registers[destinationRegisterNumber] =
                    m_registers[sourceRegisterNumber] +
                    m_registers[secondSource];
            }
            setConditionalCodes(destinationRegisterNumber);
            break;
        }
        case InstructionOpCode::AND: {
            Register destinationRegisterNumber =
                getDestinationRegisterNumber(instruction);
            Register sourceRegisterNumber =
                getSourceBaseRegisterNumber(instruction);
            if ((instruction >> 5) & 0x1) {
                uint16_t immediateValue = signExtendRetriveBits(instruction, 4, 5);
                m_registers[destinationRegisterNumber] =
                    m_registers[sourceRegisterNumber] & immediateValue;
            }
            else {
                Register secondSourceReigsterNumber =
                    static_cast<Register>(retrieveBits(instruction, 2, 3));
                m_registers[destinationRegisterNumber] =
                    m_registers[sourceRegisterNumber] &
                    m_registers[secondSourceReigsterNumber];
            }
            setConditionalCodes(destinationRegisterNumber);
            break;
        }
        case InstructionOpCode::BR: {
            uint8_t n = (instruction >> 11) & 0x1;
            uint8_t z = (instruction >> 10) & 0x1;
            uint8_t p = (instruction >> 9) & 0x1;

            // NOTE: if the offset is negative it'll be a huge number that will
            //       overflow with PC, therfore wrap it around, so that is how
            //       PC can move backwards
            uint16_t labelOffset = signExtendRetriveBits(instruction, 8, 9);

            if ((n && m_conditionalCodes.N) || (z && m_conditionalCodes.Z) ||
                (p && m_conditionalCodes.P)) {
                m_pc += labelOffset;
            }
            else if (n == 0 && z == 0 && p == 0) {
                m_pc += labelOffset;
            }
            break;
        }
        case InstructionOpCode::JMP_RET: {
            Register baseRegisterNumber =
                getSourceBaseRegisterNumber(instruction);
            m_pc = m_registers[baseRegisterNumber];
            break;
        }
        case InstructionOpCode::JSR_JSRR: {
            m_registers[R7] = m_pc;
            // is JSR
            if ((instruction >> 11) & 0x1) {
                uint16_t offset = signExtendRetriveBits(instruction, 10, 11);
                m_pc += offset;
            }
            else {
                Register baseRegisterNumber =
                    getSourceBaseRegisterNumber(instruction);
                m_pc = m_registers[baseRegisterNumber];
            }
            break;
        }
        case InstructionOpCode::LD: {
            Register destinationRegisterNumber =
                getDestinationRegisterNumber(instruction);
            uint16_t labelOffset = signExtendRetriveBits(instruction, 8, 9);
            m_registers[destinationRegisterNumber] =
                m_memory[m_pc + labelOffset];
            setConditionalCodes(destinationRegisterNumber);
            break;
        }
        case InstructionOpCode::LDI: {
            Register destinationRegisterNumber =
                getDestinationRegisterNumber(instruction);
            uint16_t labelOffset = signExtendRetriveBits(instruction, 8, 9);

            m_registers[destinationRegisterNumber] =
                m_memory[m_memory[m_pc + labelOffset]];
            setConditionalCodes(destinationRegisterNumber);
            break;
        }
        case InstructionOpCode::LDR: {
            Register destinationRegisterNumber =
                getDestinationRegisterNumber(instruction);
            Register baseRegisterNumber =
                getSourceBaseRegisterNumber(instruction);
            uint16_t immediateValue = signExtendRetriveBits(instruction, 5, 6);

            m_registers[destinationRegisterNumber] =
                m_memory[m_registers[baseRegisterNumber] + immediateValue];
            setConditionalCodes(destinationRegisterNumber);
            break;
        }
        case InstructionOpCode::LEA: {
            Register destinationRegisterNumber =
                getDestinationRegisterNumber(instruction);
            uint16_t labelOffset = signExtendRetriveBits(instruction, 8, 9);
            m_registers[destinationRegisterNumber] = m_pc + labelOffset;
            setConditionalCodes(destinationRegisterNumber);
            break;
        }
        case InstructionOpCode::NOT: {
            Register destinationRegisterNumber =
                getDestinationRegisterNumber(instruction);
            Register sourceRegisterNumber =
                getSourceBaseRegisterNumber(instruction);
            m_registers[destinationRegisterNumber] =
                ~(m_registers[sourceRegisterNumber]);
            setConditionalCodes(destinationRegisterNumber);
            break;
        }
        case InstructionOpCode::ST: {
            Register sourceRegisterNumber =
                getDestinationRegisterNumber(instruction);
            uint16_t labelOffset = signExtendRetriveBits(instruction, 8, 9);
            m_memory.write(m_pc + labelOffset,
                           m_registers[sourceRegisterNumber]);
            break;
        }
        case InstructionOpCode::STI: {
            Register sourceRegisterNumber =
                getDestinationRegisterNumber(instruction);
            uint16_t labelOffset = signExtendRetriveBits(instruction, 8, 9);
            m_memory.write(m_memory[m_pc + labelOffset],
                           m_registers[sourceRegisterNumber]);
            break;
        }
        case InstructionOpCode::STR: {
            Register sourceRegisterNumber =
                getDestinationRegisterNumber(instruction);
            Register baseRegisterNumber =
                getSourceBaseRegisterNumber(instruction);
            uint16_t labelOffset = signExtendRetriveBits(instruction, 5, 6);
            m_memory.write(m_registers[baseRegisterNumber] + labelOffset,
                           m_registers[sourceRegisterNumber]);
            break;
        }
        case InstructionOpCode::TRAP: {
            auto trapVector = static_cast<Traps>(retrieveBits(instruction, 7, 8));
            // NOTE: real implementaion should jump to trap vector table
            //       that resides in our emulator memory, and that table
            //       should point to another piece of memory that contains
            //       trap routines implementaion.
            switch (trapVector) {
            case Traps::GETC: {
                char charFromKeyboard = getchar();
                m_registers[R0] = charFromKeyboard;
                break;
            }
            case Traps::T_OUT: {
                putchar(m_registers[R0]);
                break;
            }
            case Traps::PUTS: {
                uint16_t stringPointer = m_registers[R0];
                std::string out;
                while (m_memory[stringPointer] != 0) {
                    out += m_memory[stringPointer++];
                }
                std::cout << out << '\n';
                break;
            }
            case Traps::T_IN: {
                char charFromKeyboard = getchar();
                putchar(charFromKeyboard);
                m_registers[R0] = charFromKeyboard;
                break;
            }
            case Traps::PUTSP: {
                uint16_t stringPointer = m_registers[R0];
                std::string out;
                while (m_memory[stringPointer] != 0) {
                    uint16_t twoChars = m_memory[stringPointer];
                    char char1 = retrieveBits(twoChars, 7, 8);
                    char char2 = retrieveBits(twoChars, 15, 8);
                    out += m_memory[stringPointer++];
                }
                break;
            }
            case Traps::HALT: {
                std::cout << "HALT\n";
                break;
            }
            default:
                throw std::runtime_error(
                    std::format("Trap: {} is not supported", static_cast<int>(trapVector)));
            }
            break;
        }
        case InstructionOpCode::RTI: {
            throw std::runtime_error(
                "RTI insturction is not supported by this emulator");
            break;
        }
        default: {
            throw std::runtime_error(
                std::format("Illegal instruction op code: {}"));
        }
        }
    }
    catch (...) {
        throw;
    }
}

void CPU::emulate()
{
    while (true) {
        // User is responsible for not mixing data and insturctions
        // as emulator can't differentiate insturction from
        // raw data.
        uint16_t instruction = m_memory[m_pc++];
        emulate(instruction);
        if (getOpCode(instruction) == InstructionOpCode::TRAP &&
            static_cast<Traps>(retrieveBits(instruction, 7, 8)) ==
                Traps::HALT) {
            break;
        }
    }
    restore_input_buffering();
}

void CPU::dumpMemory(uint16_t start, uint16_t size)
{
    for (uint16_t i = start; i < start + size; ++i) {
        std::cout << "memory[ " << i << " ]"
                  << " = " << m_memory[i] << std::endl;
    }
}