#include "instructions.hpp"
#include "assembler.hpp"

#include <assert.h>
#include <string>

namespace {
std::string getRegister(const std::string& lc3register)
{
    if (!lc3register.empty() && lc3register[0] == 'R') {
        return std::bitset<3>(lc3register[1] - '0').to_string();
    }
    return lc3register;
}

template <size_t N> std::string getImmediate(const std::string& immediateValue)
{
    // TODO: check for overflow
    return std::bitset<N>(std::stoi(immediateValue)).to_string();
}
} // namespace

InstructionBuilder::InstructionBuilder() : m_bitPointer(15) {}

InstructionBuilder& InstructionBuilder::set(const std::string& bits)
{
    // from left to right
    for (auto bit : bits) {
        m_instruction.set(m_bitPointer--, (bit - '0') == 1);
    }
    return *this;
}

uint16_t InstructionBuilder::instruction() const
{
    std::cout << "GENERATED INSTRUCTION: " << m_instruction << std::endl;
    return m_instruction.to_ulong();
}

Instruction::~Instruction() {}

AddInstruction::AddInstruction(const std::vector<std::string>& operands)
    : m_operands(operands)
{
}

uint16_t AddInstruction::generate()
{
    if (isImmediate()) {
        m_assembelyInstruction.set("0001")
            .set(getRegister(m_operands[0]))
            .set(getRegister(m_operands[1]))
            .set("1")
            .set(getImmediate<5>(m_operands[2]));
    }
    else {
        m_assembelyInstruction.set("0001")
            .set(getRegister(m_operands[0]))
            .set(getRegister(m_operands[1]))
            .set("000")
            .set(getRegister(m_operands[2]));
    }
    return m_assembelyInstruction.instruction();
}

bool AddInstruction::isImmediate() { return m_operands[2].front() != 'R'; }

LoadInstruction::LoadInstruction(const std::vector<std::string>& operands)
    : m_operands(operands)
{
}

uint16_t LoadInstruction::generate()
{
    std::bitset<9> labelOffset(SymbolTable::the().get(m_operands[1]));
    m_assembelyInstruction.set("0001")
        .set(getRegister(m_operands[0]))
        .set(labelOffset.to_string());
    return m_assembelyInstruction.instruction();
}

OriginDerective::OriginDerective(uint16_t origin) : m_origin(origin) {}

uint16_t OriginDerective::generate() { return m_origin; }

FillDerective::FillDerective(uint16_t value) : m_value(value) {}

uint16_t FillDerective::generate() { return m_value; }

BlkwDerective::BlkwDerective(uint16_t numberOfMemoryLocations)
    : m_numberOfMemoryLocations(numberOfMemoryLocations)
{
}

uint16_t BlkwDerective::generate()
{
    assert(true && "should not call this method");
    return -1;
}

uint16_t BlkwDerective::getNumberOfMemoryLocations() const
{
    return m_numberOfMemoryLocations;
}

StringDerective::StringDerective(const std::string& stringToWrite)
    : m_stringToWrite(stringToWrite)
{
}

uint16_t StringDerective::generate()
{
    assert(false && "should not call this method");
    return -1;
}

std::string StringDerective::getStringToWrite() const
{
    return m_stringToWrite;
}

uint16_t EndDerective::generate()
{
    assert(false && "should not call this method");
    return -1;
}