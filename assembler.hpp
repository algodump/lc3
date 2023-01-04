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

private:
    std::vector<std::shared_ptr<Instruction>> m_instructions;
    uint16_t m_programCounter;
};
