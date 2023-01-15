#include <gtest/gtest.h>

#include "../CPU.hpp"

namespace 
{
    uint16_t RESET_PC = 0;
    uint16_t INIT_PC = 42;
}

// TODO: add instruction builder that allows to
//       construct instruction from string
class CPUTests : public ::testing::Test {
      protected:
        void testAddInstruction()
        {
            // ADD R0, R1, R2 ; R0 = R1 + R2
            uint16_t instruction = 0b0001000001000010;
            cpu.m_registers[R1] = 31;
            cpu.m_registers[R2] = 42;
            cpu.emulate(instruction);
            ASSERT_EQ(cpu.m_registers[R0], cpu.m_registers[R1] + cpu.m_registers[R2]);
            ASSERT_EQ(cpu.m_conditionalCodes.P, 1);
        }

        void testAndInstruction()
        {
            // AND R0, R1, #7; R0 = R1 & R2
            uint16_t instruction = 0b0101000001100111;
            cpu.m_registers[R1] = 0b0111;
            cpu.emulate(instruction);
            ASSERT_EQ(cpu.m_registers[R0],
                      cpu.m_registers[R1] & 0x7);
            ASSERT_EQ(cpu.m_conditionalCodes.P, 1);
        }

        void testBrInsturction()
        {
            uint16_t intialPCValue = 42;
            cpu.m_pc = intialPCValue;
            uint16_t instructionBRnzp = 0b0000000001000000;
            cpu.emulate(instructionBRnzp);
            ASSERT_EQ(cpu.m_pc, intialPCValue + 64);

            // N = 0 and n = 1, don't jump
            cpu.m_pc = intialPCValue;
            uint16_t instructionBRn = 0b0000100001000000;
            cpu.emulate(instructionBRn);
            ASSERT_EQ(cpu.m_pc, intialPCValue);

            // both N = 1 and n = 1, so jump
            cpu.m_conditionalCodes.N = 1;
            cpu.m_pc = intialPCValue;
            uint16_t instructionBRnN = 0b0000100001000000;
            cpu.emulate(instructionBRnN);
            ASSERT_EQ(cpu.m_pc, intialPCValue + 64);

            // TODO: add more test if needed
        }

        void testLdInstruction() 
        {
            // LD R1, VALUE
            // value is encoded as lable location - pc
            uint16_t value = 42;
            cpu.m_pc = RESET_PC;
            cpu.m_memory[R2] = value;
            uint16_t instruction = 0b0010001000000010;
            cpu.emulate(instruction);
            ASSERT_EQ(cpu.m_registers[R1], value);
            ASSERT_EQ(cpu.m_conditionalCodes.P, 1);
        }

        void testJMP_RETInsturctions()
        {
            uint16_t registerValue = 42;
            cpu.m_registers[R7] = registerValue;
            // JMP R3
            uint16_t jmpInstruction = 0b1100000111000000;
            cpu.emulate(jmpInstruction);
            ASSERT_EQ(cpu.m_pc, registerValue);
        }

        void testJSR_JSRRInstructions()
        {
            cpu.m_pc = INIT_PC;
            // test JSR
            uint16_t jsrInstruction = 0b0100100000010000;
            cpu.emulate(jsrInstruction);
            ASSERT_EQ(cpu.m_registers[R7], INIT_PC);
            ASSERT_EQ(cpu.m_pc, INIT_PC + 16);

            // test JSRR
            cpu.m_pc = INIT_PC;
            cpu.m_registers[R2] = 31;
            uint16_t jsrrInstruction = 0b0100000010000000;
            cpu.emulate(jsrrInstruction);
            ASSERT_EQ(cpu.m_registers[R7], INIT_PC);
            ASSERT_EQ(cpu.m_pc, cpu.m_registers[R2]);
        }

      protected:
        CPU cpu;
};

TEST_F(CPUTests, AddInstruction)
{
    testAddInstruction();
}

TEST_F(CPUTests, AndInstruction)
{
    testAndInstruction();
}

TEST_F(CPUTests, LdInstruction)
{
    testLdInstruction();
}

TEST_F(CPUTests, BrInstruction)
{
    testBrInsturction();
}

TEST_F(CPUTests, JMP_RETInstructions)
{
    testJMP_RETInsturctions();
}

TEST_F(CPUTests, JSR_JSRRInstructions)
{
    testJSR_JSRRInstructions();
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}