#include "../CPU.hpp"
#include <bitset>
#include <gtest/gtest.h>

namespace {
uint16_t RESET_PC = 0x3000;
uint16_t INIT_PC = 0x3001;

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

    InstructionBuilder& set(InstructionOpCode instructionOpCode)
    {
        for (auto bit : std::bitset<4>(static_cast<uint8_t>(instructionOpCode))
                            .to_string()) {
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
                                      .set(InstructionOpCode::ADD)
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
        ASSERT_EQ(cpu.m_conditionalCodes.P, true);
    }

    void testAndInstruction()
    {
        // AND R0, R1, #7; R0 = R1 & R2
        uint16_t immediateValue = 7;
        uint16_t andInstruction = InstructionBuilder()
                                      .set(InstructionOpCode::AND)
                                      .set(R0)
                                      .set(R1)
                                      .set("1")
                                      .set(toBinaryString<5>(immediateValue))
                                      .build();
        cpu.m_registers[R1] = 8;
        cpu.emulate(andInstruction);
        ASSERT_EQ(cpu.m_registers[R0], cpu.m_registers[R1] & immediateValue);
        ASSERT_EQ(cpu.m_conditionalCodes.Z, true);
    }

    void testBrInsturction()
    {
        cpu.m_pc = INIT_PC;
        uint16_t offset = 64;
        uint16_t instructionBRnzp = InstructionBuilder()
                                        .set(InstructionOpCode::BR)
                                        .set("000")
                                        .set(toBinaryString(offset))
                                        .build();
        cpu.emulate(instructionBRnzp);
        ASSERT_EQ(cpu.m_pc, INIT_PC + offset);

        // N = 0 and n = 1, don't jump
        cpu.m_pc = INIT_PC;
        uint16_t instructionBRn = InstructionBuilder()
                                      .set(InstructionOpCode::BR)
                                      .set("100")
                                      .set(toBinaryString(offset))
                                      .build();
        cpu.emulate(instructionBRn);
        ASSERT_EQ(cpu.m_pc, INIT_PC);

        // // both N = 1 and n = 1, so jump
        cpu.m_conditionalCodes.N = true;
        cpu.m_pc = INIT_PC;
        uint16_t instructionBRnN = InstructionBuilder()
                                       .set(InstructionOpCode::BR)
                                       .set("100")
                                       .set(toBinaryString(offset))
                                       .build();
        cpu.emulate(instructionBRnN);
        ASSERT_EQ(cpu.m_pc, INIT_PC + offset);

        // jump in negative direction, use overflow trick to achive this
        uint16_t negativeOffset = -2;
        cpu.m_pc = INIT_PC;
        uint16_t instructionBR = InstructionBuilder()
                                       .set(InstructionOpCode::BR)
                                       .set("100")
                                       .set(toBinaryString(negativeOffset))
                                       .build();
        cpu.emulate(instructionBR);
        ASSERT_EQ(cpu.m_pc, INIT_PC - 2);
    }

    void testLdInstruction()
    {
        // LD R1, VALUE
        // value is encoded as lable location - pc
        //
        uint16_t offset = 1;
        uint16_t value = 42;
        cpu.m_pc = RESET_PC;
        cpu.m_memory.write(cpu.m_pc + offset, value);

        uint16_t instruction = InstructionBuilder()
                                   .set(InstructionOpCode::LD)
                                   .set(R1)
                                   .set(toBinaryString(offset))
                                   .build();
        cpu.emulate(instruction);
        ASSERT_EQ(cpu.m_registers[R1], value);
        ASSERT_EQ(cpu.m_conditionalCodes.P, true);
    }

    void testJMP_RETInsturctions()
    {
        uint16_t registerValue = 42;
        cpu.m_registers[R7] = registerValue;
        // JMP R3
        uint16_t jmpInstruction = InstructionBuilder()
                                      .set(InstructionOpCode::JMP_RET)
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
                                      .set(InstructionOpCode::JSR_JSRR)
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
                                       .set(InstructionOpCode::JSR_JSRR)
                                       .set("000")
                                       .set(R2)
                                       .set("000000")
                                       .build();
        cpu.emulate(jsrrInstruction);
        ASSERT_EQ(cpu.m_registers[R7], INIT_PC);
        ASSERT_EQ(cpu.m_pc, cpu.m_registers[R2]);
    }

    void testLdiInstruction()
    {
        Register destinationRegisterIndex = R5;
        uint16_t offset = 1;
        uint16_t value = 0;
        uint16_t valueLocation = 2;
        /*
            mem[0] = inst
            mem[1] = 2
            mem[2] = 0
        */
        cpu.m_pc = RESET_PC;
        cpu.m_memory.write(cpu.m_pc + offset, cpu.m_pc + valueLocation);
        cpu.m_memory.write(cpu.m_pc + valueLocation, value);

        uint16_t ldiInstruction = InstructionBuilder()
                                      .set(InstructionOpCode::LDI)
                                      .set(destinationRegisterIndex)
                                      .set(toBinaryString(offset))
                                      .build();
        cpu.emulate(ldiInstruction);
        ASSERT_EQ(cpu.m_registers[destinationRegisterIndex], 0);
        ASSERT_EQ(cpu.m_conditionalCodes.Z, true);
    }

    void testLdrInstruction()
    {
        Register destinationRegisterIndex = R2;
        Register baseRegisterIndex = R1;
        uint16_t offset = 11;
        uint16_t value = 31;
        uint16_t ldrInstruction = InstructionBuilder()
                                      .set(InstructionOpCode::LDR)
                                      .set(destinationRegisterIndex)
                                      .set(baseRegisterIndex)
                                      .set(toBinaryString<6>(offset))
                                      .build();
        cpu.m_pc = RESET_PC;                             
        cpu.m_registers[baseRegisterIndex] = cpu.m_pc;
        cpu.m_memory.write(cpu.m_pc + offset, value);

        cpu.emulate(ldrInstruction);
        ASSERT_EQ(cpu.m_registers[destinationRegisterIndex], value);
        ASSERT_EQ(cpu.m_conditionalCodes.P, true);
    }

    void testLeaInstruction()
    {
        Register destinationRegisterNumber = R5;
        uint16_t offset = 72;
        uint16_t leaInstruction = InstructionBuilder()
                                      .set(InstructionOpCode::LEA)
                                      .set(destinationRegisterNumber)
                                      .set(toBinaryString(offset))
                                      .build();
        cpu.m_pc = INIT_PC;
        cpu.emulate(leaInstruction);
        ASSERT_EQ(cpu.m_registers[destinationRegisterNumber], offset + INIT_PC);
    }

    void testNotInstruction()
    {
        Register destinationRegisterNumber = R0;
        Register sourceRegisterNumber = R1;
        uint16_t notInstruction = InstructionBuilder()
                                      .set(InstructionOpCode::NOT)
                                      .set(destinationRegisterNumber)
                                      .set(sourceRegisterNumber)
                                      .build();
        cpu.m_registers[sourceRegisterNumber] = 0b1010101010101010;
        cpu.emulate(notInstruction);
        ASSERT_EQ(cpu.m_registers[destinationRegisterNumber],
                  (uint16_t)~cpu.m_registers[sourceRegisterNumber]);
        ASSERT_EQ(cpu.m_conditionalCodes.P, true);
    }

    void testStInstruction()
    {
        Register sourceRegisterNumber = R0;
        uint16_t offset = 212;
        std::string test = toBinaryString(offset);
        uint16_t stInstruction = InstructionBuilder()
                                     .set(InstructionOpCode::ST)
                                     .set(sourceRegisterNumber)
                                     .set(toBinaryString(offset))
                                     .build();
        cpu.m_pc = RESET_PC;
        cpu.m_registers[sourceRegisterNumber] = 32;
        cpu.emulate(stInstruction);
        ASSERT_EQ(cpu.m_memory[cpu.m_pc + offset],
                  cpu.m_registers[sourceRegisterNumber]);
    }

    void testStiInstruction()
    {
        Register sourceRegisterNumber = R0;
        uint16_t offset = 31;
        uint16_t stInstruction = InstructionBuilder()
                                     .set(InstructionOpCode::STI)
                                     .set(sourceRegisterNumber)
                                     .set(toBinaryString(offset))
                                     .build();
        cpu.m_pc = RESET_PC;
        cpu.m_registers[sourceRegisterNumber] = 1024;
        cpu.m_memory.write(cpu.m_pc + offset, cpu.m_pc + 2);
        cpu.emulate(stInstruction);
        ASSERT_EQ(cpu.m_memory[cpu.m_pc + 2],
                  cpu.m_registers[sourceRegisterNumber]);
    }

    void testStrInstruction() {
        Register sourceRegisterNumber = R0;
        Register baseRegisterNumber = R1;
        uint16_t offset = 13;

        uint16_t strInstruction = InstructionBuilder()
                                    .set(InstructionOpCode::STR)
                                    .set(sourceRegisterNumber)
                                    .set(baseRegisterNumber)
                                    .set(toBinaryString<6>(offset))
                                    .build();
        cpu.m_registers[baseRegisterNumber] = 0x3000;
        cpu.m_registers[sourceRegisterNumber] = 42;
        cpu.emulate(strInstruction);
        ASSERT_EQ(cpu.m_memory[cpu.m_registers[baseRegisterNumber] + offset],
                  cpu.m_registers[sourceRegisterNumber]);
    }

  protected:
    CPU cpu;
};

TEST_F(CPUTests, ADD) { testAddInstruction(); }

TEST_F(CPUTests, AND) { testAndInstruction(); }

TEST_F(CPUTests, LD) { testLdInstruction(); }

TEST_F(CPUTests, BR) { testBrInsturction(); }

TEST_F(CPUTests, JMP_RET) { testJMP_RETInsturctions(); }

TEST_F(CPUTests, JSR_JSRR) { testJSR_JSRRInstructions(); }

TEST_F(CPUTests, LDI) { testLdiInstruction(); }

TEST_F(CPUTests, LDR) { testLdrInstruction(); }

TEST_F(CPUTests, LEA) { testLeaInstruction(); }

TEST_F(CPUTests, NOT) { testNotInstruction(); }

TEST_F(CPUTests, ST) { testStInstruction(); }

TEST_F(CPUTests, STI) { testStiInstruction(); }

TEST_F(CPUTests, STR) { testStrInstruction(); }

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}