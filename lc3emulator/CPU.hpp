#pragma once

#include "lc3memory.hpp"

#include <array>
#include <string>

enum class InstructionOpCode : uint8_t {
    BR = 0b0000,
    ADD = 0b0001,
    LD = 0b0010,
    ST = 0b0011,
    JSR_JSRR = 0b0100,
    AND = 0b0101,
    LDR = 0b0110,
    STR = 0b0111,
    RTI = 0b1000,
    NOT = 0b1001,
    LDI = 0b1010,
    STI = 0b1011,
    JMP_RET = 0b1100,
    NON = 0b1101,
    LEA = 0b1110,
    TRAP = 0b1111,
};

enum class Traps : uint8_t {
    GETC = 0x20,
    OUT = 0x21,
    PUTS = 0x22,
    IN = 0x23,
    PUTSP = 0x24,
    HALT = 0x25
};

// TODO: add to string conversion
enum class StatusCode : uint8_t {
    SUCCESS,
    HALTED,
};

enum Register {
    R0 = 0, R1 = 1, R2 = 2, R3 = 3, R4 = 4, R5 = 5, R6 = 6, R7 = 7
};

class Instruction {
  public:
    Instruction(uint16_t instruction) : m_instruction(instruction) {}
    uint16_t getBits(uint8_t from, uint8_t to);

  private:
    uint16_t m_instruction;
};


class CPU {
  public:

    static constexpr uint8_t NUMBER_OF_REGISTERS = 8;
    using Registers = std::array<int16_t, NUMBER_OF_REGISTERS>;

  public:
    CPU();
    bool load(const std::string& fileToRun);
    StatusCode emulate();
    StatusCode emulate(uint16_t instruction);

  private:
    void dumpMemory(int16_t start, int16_t size);
    InstructionOpCode getOpCode(int16_t instruction) const;
    void setConditionalCodes(Register destinationRegister);

  private:
    Memory m_memory;
    Registers m_registers;
    uint16_t m_pc;
    struct ConditionalCode {
        bool N;
        bool Z;
        bool P;
    } m_conditionalCodes;
    
    friend class CPUTests;
};