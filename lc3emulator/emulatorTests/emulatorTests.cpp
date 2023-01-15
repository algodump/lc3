#include "../CPU.hpp"
#include <bitset>
#include <gtest/gtest.h>


namespace {
uint16_t RESET_PC = 0;
uint16_t INIT_PC = 42;

// copied from lc3assembler/instructions.hpp
class InstructionBuilder {
  public:
    InstructionBuilder() : m_bitPointer(15) {}
    InstructionBuilder& set(const std::string& bits)
    {
        for (auto bit : bits) {
            m_instruction.set(m_bitPointer--, (bit - '0') == 1);
        }
        return *this;
    }

    InstructionBuilder& set(Register registerNumber)
    {
        for (auto bit : std::bitset<3>(registerNumber).to_string()) {
            m_instruction.set(m_bitPointer--, (bit - '0') == 1);
        }
        return *this;
    }

    InstructionBuilder& set(char bit)
    {
        m_instruction.set(m_bitPointer--, (bit - '0') == 1);
        return *this;
    }

    uint16_t build() const { return m_instruction.to_ulong(); }

  private:
    uint8_t m_bitPointer;
    std::bitset<16> m_instruction;
};

template <uint16_t bitcount = 9>
static std::string toBinaryString(uint16_t number)
{
    return std::bitset<bitcount>(number).to_string();
}
} // namespace

class CPUTests : public ::testing::Test {
  protected:
    void testAddInstruction()
    {
        // ADD R0, R1, R2 ; R0 = R1 + R2
        uint16_t addInstruction = InstructionBuilder()
                                      .set("0001")
                                      .set(R0)
                                      .set(R1)
                                      .set("000")
                                      .set(R2)
                                      .build();
        cpu.m_registers[R1] = 31;
        cpu.m_registers[R2] = 42;
        cpu.emulate(addInstruction);
        ASSERT_EQ(cpu.m_registers[R0],
                  cpu.m_registers[R1] + cpu.m_registers[R2]);
        ASSERT_EQ(cpu.m_conditionalCodes.P, 1);
    }

    void testAndInstruction()
    {
        // AND R0, R1, #7; R0 = R1 & R2
        uint16_t immediateValue = 7;
        uint16_t andInstruction = InstructionBuilder()
                                      .set("0101")
                                      .set(R0)
                                      .set(R1)
                                      .set("1")
                                      .set(toBinaryString<5>(immediateValue))
                                      .build();
        cpu.m_registers[R1] = 8;
        cpu.emulate(andInstruction);
        ASSERT_EQ(cpu.m_registers[R0], cpu.m_registers[R1] & immediateValue);
        ASSERT_EQ(cpu.m_conditionalCodes.P, 1);
    }

    void testBrInsturction()
    {
        cpu.m_pc = INIT_PC;
        uint16_t offset = 64;
        uint16_t instructionBRnzp = InstructionBuilder()
                                        .set("0000")
                                        .set("000")
                                        .set(toBinaryString(offset))
                                        .build();
        cpu.emulate(instructionBRnzp);
        ASSERT_EQ(cpu.m_pc, INIT_PC + offset);

        // N = 0 and n = 1, don't jump
        cpu.m_pc = INIT_PC;
        uint16_t instructionBRn = InstructionBuilder()
                                      .set("0000")
                                      .set("100")
                                      .set(toBinaryString(offset))
                                      .build();
        cpu.emulate(instructionBRn);
        ASSERT_EQ(cpu.m_pc, INIT_PC);

        // // both N = 1 and n = 1, so jump
        cpu.m_conditionalCodes.N = 1;
        cpu.m_pc = INIT_PC;
        uint16_t instructionBRnN = InstructionBuilder()
                                       .set("0000")
                                       .set("100")
                                       .set(toBinaryString(offset))
                                       .build();
        cpu.emulate(instructionBRnN);
        ASSERT_EQ(cpu.m_pc, INIT_PC + offset);

        // TODO: add more test if needed
    }

    void testLdInstruction()
    {
        // LD R1, VALUE
        // value is encoded as lable location - pc
        //
        uint16_t offset = 1;
        uint16_t value = 42;
        cpu.m_pc = RESET_PC;
        cpu.m_memory[offset] = value;

        uint16_t instruction = InstructionBuilder()
                                   .set("0010")
                                   .set(R1)
                                   .set(toBinaryString(offset))
                                   .build();
        cpu.emulate(instruction);
        ASSERT_EQ(cpu.m_registers[R1], value);
        ASSERT_EQ(cpu.m_conditionalCodes.P, 1);
    }

    void testJMP_RETInsturctions()
    {
        uint16_t registerValue = 42;
        cpu.m_registers[R7] = registerValue;
        // JMP R3
        uint16_t jmpInstruction = InstructionBuilder()
                                      .set("1100")
                                      .set("000")
                                      .set(R7)
                                      .set("000000")
                                      .build();
        ;
        cpu.emulate(jmpInstruction);
        ASSERT_EQ(cpu.m_pc, registerValue);
    }

    void testJSR_JSRRInstructions()
    {
        cpu.m_pc = INIT_PC;
        uint16_t offset = 16;
        // test JSR
        uint16_t jsrInstruction = InstructionBuilder()
                                      .set("0100")
                                      .set('1')
                                      .set(toBinaryString<11>(offset))
                                      .build();
        cpu.emulate(jsrInstruction);
        ASSERT_EQ(cpu.m_registers[R7], INIT_PC);
        ASSERT_EQ(cpu.m_pc, INIT_PC + offset);

        // test JSRR
        cpu.m_pc = INIT_PC;
        cpu.m_registers[R2] = 31;
        uint16_t jsrrInstruction = InstructionBuilder()
                                       .set("0100")
                                       .set("000")
                                       .set(R2)
                                       .set("000000")
                                       .build();
        cpu.emulate(jsrrInstruction);
        ASSERT_EQ(cpu.m_registers[R7], INIT_PC);
        ASSERT_EQ(cpu.m_pc, cpu.m_registers[R2]);
    }

  protected:
    CPU cpu;
};

TEST_F(CPUTests, AddInstruction) { testAddInstruction(); }

TEST_F(CPUTests, AndInstruction) { testAndInstruction(); }

TEST_F(CPUTests, LdInstruction) { testLdInstruction(); }

TEST_F(CPUTests, BrInstruction) { testBrInsturction(); }

TEST_F(CPUTests, JMP_RETInstructions) { testJMP_RETInsturctions(); }

TEST_F(CPUTests, JSR_JSRRInstructions) { testJSR_JSRRInstructions(); }

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}