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

std::string getRegister(uint16_t lc3register)
{
    return std::bitset<3>(lc3register).to_string();
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

InstructionBuilder& InstructionBuilder::set(char bit)
{
    m_instruction.set(m_bitPointer--, (bit - '0') == 1);
    return *this;
}

uint16_t InstructionBuilder::instruction() const
{
    std::cout << "GENERATED INSTRUCTION: " << m_instruction << std::endl;
    return m_instruction.to_ulong();
}

std::string Instruction::opcode() const { return ""; }

Instruction::~Instruction() {}

bool isImmediate(const std::string& thirdOperand)
{
    return thirdOperand.front() != 'R';
}

AddInstruction::AddInstruction(const std::vector<std::string>& operands)
    : m_operands(operands)
{
}

uint16_t AddInstruction::generate()
{
    if (isImmediate(m_operands[2])) {
        m_assembelyInstruction.set(opcode())
            .set(getRegister(m_operands[0]))
            .set(getRegister(m_operands[1]))
            .set('1')
            .set(getImmediate<5>(m_operands[2]));
    }
    else {
        m_assembelyInstruction.set(opcode())
            .set(getRegister(m_operands[0]))
            .set(getRegister(m_operands[1]))
            .set("000")
            .set(getRegister(m_operands[2]));
    }
    return m_assembelyInstruction.instruction();
}

std::string AddInstruction::opcode() const { return "0001"; }

LoadInstruction::LoadInstruction(const std::vector<std::string>& operands)
    : m_operands(operands)
{
}

uint16_t LoadInstruction::generate()
{
    std::bitset<9> labelOffset(SymbolTable::the().get(m_operands[1]));
    m_assembelyInstruction.set(opcode())
        .set(getRegister(m_operands[0]))
        .set(labelOffset.to_string());
    return m_assembelyInstruction.instruction();
}

std::string LoadInstruction::opcode() const { return "0010"; }

AndInstruction::AndInstruction(const std::vector<std::string>& operands)
    : m_operands(operands)
{
}

uint16_t AndInstruction::generate()
{
    if (isImmediate(m_operands[2])) {
        m_assembelyInstruction.set(opcode())
            .set(getRegister(m_operands[0]))
            .set(getRegister(m_operands[1]))
            .set('1')
            .set(getImmediate<5>(m_operands[2]));
    }
    else {
        m_assembelyInstruction.set(opcode())
            .set(getRegister(m_operands[0]))
            .set(getRegister(m_operands[1]))
            .set("000")
            .set(getRegister(m_operands[2]));
    }
    return m_assembelyInstruction.instruction();
}

std::string AndInstruction::opcode() const { return "0101"; }

BrInstruction::BrInstruction(const std::string& conditionalCodes,
                             const std::string& label)
    : m_conditionalCodes(conditionalCodes), m_label(label)
{
}

uint16_t BrInstruction::generate()
{
    std::bitset<9> labelOffset(SymbolTable::the().get(m_label));
    // NOTE: BRnzp and BR are the same, so if either all codes are set or none,
    //       reuslt must be the same
    if (m_conditionalCodes.empty() || m_conditionalCodes.size() == 3) {
        // clang-format off
           m_assembelyInstruction.set(opcode())
            .set('1')
            .set('1')
            .set('1')
            .set(labelOffset.to_string());
    }
    else {
        char n = m_conditionalCodes.find('n') != std::string::npos ? '1' : '0';
        char z = m_conditionalCodes.find('z') != std::string::npos ? '1' : '0';
        char p = m_conditionalCodes.find('p') != std::string::npos ? '1' : '0';
        m_assembelyInstruction.set(opcode())
            .set(n)
            .set(z)
            .set(p)
            .set(labelOffset.to_string());
    } // clang-format on
    return m_assembelyInstruction.instruction();
}

std::string BrInstruction::opcode() const { return "0000"; }

JmpInsturction::JmpInsturction(uint16_t baseRegister)
    : m_baseRegister(baseRegister)
{
}

uint16_t JmpInsturction::generate()
{
    m_assembelyInstruction.set(opcode())
        .set("000")
        .set(getRegister(m_baseRegister))
        .set("000000");
    return m_assembelyInstruction.instruction();
}

std::string JmpInsturction::opcode() const { return "1100"; }

uint16_t RetInstruction::generate() { return JmpInsturction(7).generate(); }

std::string RetInstruction::opcode() const { return "1100"; }

JsrInstruction::JsrInstruction(const std::string& label) : m_label(label) {}

uint16_t JsrInstruction::generate() 
{
    std::bitset<11>labelOffset(SymbolTable::the().get(m_label));
    m_assembelyInstruction.set(opcode())
        .set('1')
        .set(labelOffset.to_string());
    return m_assembelyInstruction.instruction();
}

std::string JsrInstruction::opcode() const
{
    return "0100";
}

JsrrInstruction::JsrrInstruction(uint16_t baseRgister)
    : m_baseRegister(baseRgister)
{
}

uint16_t JsrrInstruction::generate() 
{
    m_assembelyInstruction.set(opcode())
        .set('0')
        .set("00")
        .set(getRegister(m_baseRegister))
        .set("000000");
    return m_assembelyInstruction.instruction();
}

std::string JsrrInstruction::opcode() const 
{
    return "0100";
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