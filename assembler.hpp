#pragma once

#include "reader.hpp"
#include <fstream>

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
    Assembler(std::vector<std::shared_ptr<Instruction>>& instructions);
    void gnenerate(Writer& writer);

    template<uint16_t bitcount = 9>
    static std::string toBinaryString(uint16_t number) {
        return std::bitset<bitcount>(number).to_string();
    }

  private:
    std::vector<std::shared_ptr<Instruction>> m_instructions;
};
