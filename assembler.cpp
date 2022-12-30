#include "assembler.hpp"

#include <bitset>
#include <assert.h>

Writer::Writer(const std::string &filename) : m_outpuStream(filename, std::ios::binary)
{
}

void Writer::wirte(uint16_t instruction)
{
    m_outpuStream.write(reinterpret_cast<char*>(&instruction), sizeof(instruction));
}


Assembler::Assembler(const std::vector<std::shared_ptr<Instruction>> &instructions) : m_instructions(instructions) {}

void Assembler::gnenerate(Writer& writer) 
{
    for (auto& instruction : m_instructions) {
        auto binaryInstruction = instruction->generate();
        writer.wirte(binaryInstruction);
    }
}