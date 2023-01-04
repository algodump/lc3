#include "assembler.hpp"

#include <assert.h>
#include <bitset>

Writer::Writer(const std::string& filename)
    : m_outpuStream(filename, std::ios::binary)
{
}

void Writer::write(uint16_t instruction)
{
    m_outpuStream.write(reinterpret_cast<char*>(&instruction),
                        sizeof(instruction));
}

void Writer::write(const std::string& data)
{
    m_outpuStream.write(data.c_str(), std::streamsize(data.size()));
}

Assembler::Assembler(std::vector<std::shared_ptr<Instruction>>& instructions)
    : m_instructions(instructions)
{
}

void Assembler::gnenerate(Writer& writer)
{
    for (auto& instruction : m_instructions) {
        auto binaryInstruction = instruction->generate();
        writer.write(binaryInstruction);
    }
}