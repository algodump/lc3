#pragma once

#include "reader.hpp"
#include <fstream>

class Writer {
public:
    Writer(const std::string& filename);
    void wirte(uint16_t instruction);

private:
    std::ofstream m_outpuStream;
};

class Assembler {
public:
    Assembler(const std::vector<std::shared_ptr<Instruction>>& instructions);
    void gnenerate(Writer& writer);

private:
    std::vector<std::shared_ptr<Instruction>> m_instructions;
};
