#pragma once

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
    static constexpr uint16_t MEMORY_CAPACITY =
        std::numeric_limits<uint16_t>::max();
    static constexpr uint8_t NUMBER_OF_REGISTERS = 8;
    using Registers = std::array<int16_t, NUMBER_OF_REGISTERS>;
    using Memory = std::array<uint16_t, MEMORY_CAPACITY>;

  public:
    CPU();
    bool load(const std::string& fileToRun);
    bool emulate();
    bool emulate(uint16_t instruction);

  private:
    void dumpMemory(uint16_t start, uint16_t size);
    InstructionOpCode getOpCode(uint16_t instruction) const;
    void setConditionalCodes(Register destinationRegister);

  private:
    Memory m_memory;
    Registers m_registers;
    uint16_t m_pc;
    struct ConditionalCode {
        uint8_t N;
        uint8_t Z;
        uint8_t P;
    } m_conditionalCodes;
    
    friend class CPUTests;
};