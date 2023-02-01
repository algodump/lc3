#include "instructions.hpp"
#include "assembler.hpp"

#include <assert.h>
#include <string>

namespace {
std::string convertRegisterToBinary(uint8_t lc3register)
{
    return Assembler::toBinaryString<3>(lc3register);
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
#ifdef DEBUG
    std::map<uint16_t, std::string> instructions = {
        {0b0000, "BR"},      {0b0001, "ADD"},      {0b0010, "LD"},
        {0b0011, "ST"},      {0b0100, "JSR_JSRR"}, {0b0101, "AND"},
        {0b0110, "LDR"},     {0b0111, "STR"},      {0b1000, "RTI"},
        {0b1001, "NOT"},     {0b1010, "LDI"},      {0b1011, "STI"},
        {0b1100, "JMP_RET"}, {0b1101, "NON"},      {0b1110, "LEA"},
        {0b1111, "TRAP"}};
    std::cout << "GENERATING: "
              << instructions.at(m_instruction.to_ulong() >> 12) << std::endl;
#endif
    return m_instruction.to_ulong();
}

std::string Instruction::opcode() const { return ""; }

Instruction::~Instruction() {}

AddInstruction::AddInstruction(uint8_t destinationRegister,
                               uint8_t source1Register,
                               uint8_t source2RegisterOrImmediate,
                               bool isImmediate)
    : m_destinationRegister(destinationRegister),
      m_source1Register(source1Register), m_isImmediate(isImmediate)
{
    if (isImmediate) {
        m_immediateValue = source2RegisterOrImmediate;
    }
    else {
        m_source2Register = source2RegisterOrImmediate;
    }
}

uint16_t AddInstruction::generate(uint16_t)
{
    if (m_isImmediate) {
        m_assembelyInstruction.set(opcode())
            .set(convertRegisterToBinary(m_destinationRegister))
            .set(convertRegisterToBinary(m_source1Register))
            .set('1')
            .set(Assembler::toBinaryString<5>(m_immediateValue));
    }
    else {
        m_assembelyInstruction.set(opcode())
            .set(convertRegisterToBinary(m_destinationRegister))
            .set(convertRegisterToBinary(m_source1Register))
            .set("000")
            .set(convertRegisterToBinary(m_source2Register));
    }
    return m_assembelyInstruction.instruction();
}

std::string AddInstruction::opcode() const { return "0001"; }

AndInstruction::AndInstruction(uint8_t destinationRegister,
                               uint8_t source1Register,
                               uint8_t source2RegisterOrImmediate,
                               bool isImmediate)
    : m_destinationRegister(destinationRegister),
      m_source1Register(source1Register), m_isImmediate(isImmediate)
{
    if (isImmediate) {
        m_immediateValue = source2RegisterOrImmediate;
    }
    else {
        m_source2Register = source2RegisterOrImmediate;
    }
}

uint16_t AndInstruction::generate(uint16_t)
{
    if (m_isImmediate) {
        m_assembelyInstruction.set(opcode())
            .set(convertRegisterToBinary(m_destinationRegister))
            .set(convertRegisterToBinary(m_source1Register))
            .set('1')
            .set(Assembler::toBinaryString<5>(m_immediateValue));
    }
    else {
        m_assembelyInstruction.set(opcode())
            .set(convertRegisterToBinary(m_destinationRegister))
            .set(convertRegisterToBinary(m_source1Register))
            .set("000")
            .set(convertRegisterToBinary(m_source2Register));
    }
    return m_assembelyInstruction.instruction();
}

std::string AndInstruction::opcode() const { return "0101"; }

BrInstruction::BrInstruction(const std::string& conditionalCodes,
                             const std::string& label)
    : m_conditionalCodes(conditionalCodes), m_label(label)
{
}

uint16_t BrInstruction::generate(uint16_t currentPC)
{
    auto labelOffset = Assembler::getBinaryOffsetToJumpTo(m_label, currentPC);
    // NOTE: BRnzp and BR are the same, so if either all codes are set or none,
    //       reuslt must be the same
    if (m_conditionalCodes.empty() || m_conditionalCodes.size() == 3) {
        // clang-format off
           m_assembelyInstruction.set(opcode())
            .set('1')
            .set('1')
            .set('1')
            .set(labelOffset);
    }
    else {
        char n = m_conditionalCodes.find('n') != std::string::npos ? '1' : '0';
        char z = m_conditionalCodes.find('z') != std::string::npos ? '1' : '0';
        char p = m_conditionalCodes.find('p') != std::string::npos ? '1' : '0';
        m_assembelyInstruction.set(opcode())
            .set(n)
            .set(z)
            .set(p)
            .set(labelOffset);
    } // clang-format on
    return m_assembelyInstruction.instruction();
}

std::string BrInstruction::opcode() const { return "0000"; }

JmpInsturction::JmpInsturction(uint8_t baseRegister)
    : m_baseRegister(baseRegister)
{
}

uint16_t JmpInsturction::generate(uint16_t)
{
    m_assembelyInstruction.set(opcode())
        .set("000")
        .set(convertRegisterToBinary(m_baseRegister))
        .set("000000");
    return m_assembelyInstruction.instruction();
}

std::string JmpInsturction::opcode() const { return "1100"; }

uint16_t RetInstruction::generate(uint16_t currentPC)
{
    return JmpInsturction(7).generate(currentPC);
}

std::string RetInstruction::opcode() const { return "1100"; }

JsrInstruction::JsrInstruction(const std::string& labelOrOffset)
    : m_labelOrOffset(labelOrOffset)
{
}

uint16_t JsrInstruction::generate(uint16_t currentPC)
{
    auto offsetToJump =
        Assembler::getBinaryOffsetToJumpTo<11>(m_labelOrOffset, currentPC);
    m_assembelyInstruction.set(opcode()).set('1').set(offsetToJump);
    return m_assembelyInstruction.instruction();
}

std::string JsrInstruction::opcode() const { return "0100"; }

JsrrInstruction::JsrrInstruction(uint8_t baseRgister)
    : m_baseRegister(baseRgister)
{
}

uint16_t JsrrInstruction::generate(uint16_t currentPC)
{
    m_assembelyInstruction.set(opcode())
        .set('0')
        .set("00")
        .set(convertRegisterToBinary(m_baseRegister))
        .set("000000");
    return m_assembelyInstruction.instruction();
}

std::string JsrrInstruction::opcode() const { return "0100"; }

LdInstruction::LdInstruction(uint8_t destinationRegister,
                             const std::string& labelOrOffset)
    : m_destinationRegister(destinationRegister), m_labelOrOffset(labelOrOffset)
{
}

uint16_t LdInstruction::generate(uint16_t currentPC)
{
    auto offsetToJump =
        Assembler::getBinaryOffsetToJumpTo(m_labelOrOffset, currentPC);
    m_assembelyInstruction.set(opcode())
        .set(convertRegisterToBinary(m_destinationRegister))
        .set(offsetToJump);
    return m_assembelyInstruction.instruction();
}

std::string LdInstruction::opcode() const { return "0010"; }

LdiInsturction::LdiInsturction(uint8_t destinationRegister,
                               const std::string& label)
    : m_destinationRegister(destinationRegister), m_labelOrOffset(label)
{
}

uint16_t LdiInsturction::generate(uint16_t currentPC)
{
    auto offsetToJump =
        Assembler::getBinaryOffsetToJumpTo(m_labelOrOffset, currentPC);
    m_assembelyInstruction.set(opcode())
        .set(convertRegisterToBinary(m_destinationRegister))
        .set(offsetToJump);
    return m_assembelyInstruction.instruction();
}

std::string LdiInsturction::opcode() const { return "1010"; }

LdrInstruction::LdrInstruction(uint8_t destinationRegister,
                               uint8_t baseRegister,
                               const std::string& labelOrOffset)
    : m_destinationRegister(destinationRegister), m_baseRegister(baseRegister),
      m_labelOrOffset(labelOrOffset)
{
}

uint16_t LdrInstruction::generate(uint16_t currentPC)
{
    auto offsetToJump =
        Assembler::getBinaryOffsetToJumpTo<6>(m_labelOrOffset, currentPC);
    m_assembelyInstruction.set(opcode())
        .set(convertRegisterToBinary(m_destinationRegister))
        .set(convertRegisterToBinary(m_baseRegister))
        .set(offsetToJump);
    return m_assembelyInstruction.instruction();
}

std::string LdrInstruction::opcode() const { return "0110"; }

LeaInstruction::LeaInstruction(uint8_t destinationRegister,
                               const std::string& labelOrOffset)
    : m_destinationRegister(destinationRegister), m_labelOrOffset(labelOrOffset)
{
}

uint16_t LeaInstruction::generate(uint16_t currentPC)
{
    auto offsetToJump =
        Assembler::getBinaryOffsetToJumpTo(m_labelOrOffset, currentPC);
    m_assembelyInstruction.set(opcode())
        .set(convertRegisterToBinary(m_destinationRegister))
        .set(offsetToJump);
    return m_assembelyInstruction.instruction();
}

std::string LeaInstruction::opcode() const { return "1110"; }

StInstruction::StInstruction(uint8_t sourceRegister,
                             const std::string& labelOrOffset)
    : m_sourceRegister(sourceRegister), m_labelOrOffset(labelOrOffset)
{
}

uint16_t StInstruction::generate(uint16_t currentPC)
{
    auto offsetToJump =
        Assembler::getBinaryOffsetToJumpTo(m_labelOrOffset, currentPC);
    m_assembelyInstruction.set(opcode())
        .set(convertRegisterToBinary(m_sourceRegister))
        .set(offsetToJump);
    return m_assembelyInstruction.instruction();
}

std::string StInstruction::opcode() const { return "0011"; }

StiInstruction::StiInstruction(uint8_t sourceRegister,
                               const std::string& labelOrOffset)
    : m_sourceRegister(sourceRegister), m_labelOrOffset(labelOrOffset)
{
}

uint16_t StiInstruction::generate(uint16_t currentPC)
{
    auto offsetToJump =
        Assembler::getBinaryOffsetToJumpTo(m_labelOrOffset, currentPC);
    m_assembelyInstruction.set(opcode())
        .set(convertRegisterToBinary(m_sourceRegister))
        .set(offsetToJump);
    return m_assembelyInstruction.instruction();
}

std::string StiInstruction::opcode() const { return "1011"; }

NotInstruction::NotInstruction(uint8_t destinationRegister,
                               uint8_t sourceRegister)
    : m_destinationRegister(destinationRegister),
      m_sourceRegister(sourceRegister)
{
}

uint16_t NotInstruction::generate(uint16_t)
{
    m_assembelyInstruction.set(opcode())
        .set(convertRegisterToBinary(m_destinationRegister))
        .set(convertRegisterToBinary(m_sourceRegister))
        .set("111111");
    return m_assembelyInstruction.instruction();
}

std::string NotInstruction::opcode() const { return "1001"; }

uint16_t RtiInstruction::generate(uint16_t)
{
    m_assembelyInstruction.set(opcode()).set("000000000000");
    return m_assembelyInstruction.instruction();
}

std::string RtiInstruction::opcode() const { return "1000"; };

TrapInstruction::TrapInstruction(uint8_t trapVector) : m_trapVector(trapVector)
{
}

uint16_t TrapInstruction::generate(uint16_t)
{
    m_assembelyInstruction.set("1111").set("0000").set(
        Assembler::toBinaryString<8>(m_trapVector));
    return m_assembelyInstruction.instruction();
}

std::string TrapInstruction::opcode() const { return "1111"; }

StrInstruction::StrInstruction(uint8_t sourceRegister, uint8_t baseRegister,
                               const std::string& labelOrOffset)
    : m_sourceRegister(sourceRegister), m_baseRegister(baseRegister),
      m_labelOrOffset(labelOrOffset)
{
}

uint16_t StrInstruction::generate(uint16_t currentPC)
{
    auto offset =
        Assembler::getBinaryOffsetToJumpTo<6>(m_labelOrOffset, currentPC);
    m_assembelyInstruction.set(opcode())
        .set(convertRegisterToBinary(m_sourceRegister))
        .set(convertRegisterToBinary(m_baseRegister))
        .set(offset);
    return m_assembelyInstruction.instruction();
}

std::string StrInstruction::opcode() const { return "0111"; }

OriginDerective::OriginDerective(uint16_t origin) : m_origin(origin) {}

uint16_t OriginDerective::generate(uint16_t) { return m_origin; }

FillDerective::FillDerective(uint16_t value) : m_value(value) {}

uint16_t FillDerective::generate(uint16_t) { return m_value; }

BlkwDerective::BlkwDerective(uint16_t numberOfMemoryLocations)
    : m_numberOfMemoryLocations(numberOfMemoryLocations)
{
}

uint16_t BlkwDerective::generate(uint16_t)
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

uint16_t StringDerective::generate(uint16_t)
{
    assert(false && "should not call this method");
    return -1;
}

std::string StringDerective::getStringToWrite() const
{
    return m_stringToWrite;
}

uint16_t EndDerective::generate(uint16_t)
{
    assert(false && "should not call this method");
    return -1;
}