#pragma once

#include "reader.hpp"
#include <fstream>
#include <assert.h>

class Writer {
  public:
    Writer(const std::string& filename);
    void write(uint16_t instruction);
    void write(const std::string& data);

  private:
    std::ofstream m_outpuStream;
};

class Assembler {
  public:
    Assembler(std::vector<InstructionWithAddress>& instructions);
    void gnenerate(Writer& writer);

    template <uint16_t bitcount = 9>
    static std::string toBinaryString(uint16_t number)
    {
        return std::bitset<bitcount>(number).to_string();
    }

    static bool isImmediate(const std::string& thirdOperand)
    {
        return thirdOperand.front() == '#';
    }

    template <uint16_t bitcount = 9>
    static std::string getBinaryOffsetToJumpTo(const std::string& labelOrOffset, uint16_t currentPC)
    {
        if (isImmediate(labelOrOffset)) {
            return toBinaryString<bitcount>(std::stoi(labelOrOffset.substr(1)));
        }
        auto labelLocation = SymbolTable::the().get(labelOrOffset);
        uint16_t offset = labelLocation - currentPC - 1;
        return Assembler::toBinaryString<bitcount>(offset);
    }

  private:
    std::vector<InstructionWithAddress> m_instructions;
};
