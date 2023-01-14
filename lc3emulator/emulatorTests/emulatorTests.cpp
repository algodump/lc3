#include <gtest/gtest.h>

#include "../CPU.hpp"

class CPUTests : public ::testing::Test {
      protected:
        void testAddInstruction()
        {
            // ADD R0, R1, R2 ; R0 = R1 + R2
            uint16_t instruction = 0b0001000001000010;
            cpu.m_registers[1] = 31;
            cpu.m_registers[2] = 42;
            cpu.emulate(instruction);
            ASSERT_EQ(cpu.m_registers[0], cpu.m_registers[1] + cpu.m_registers[2]);
        }

        void testLoadInstruction() 
        {
            // LD R1, VALUE
            // value is encoded as lable location - pc
            uint16_t value = 42;
            cpu.m_pc = 0;
            cpu.m_memory[2] = value;
            uint16_t instruction = 0b0010001000000010;
            cpu.emulate(instruction);
            ASSERT_EQ(cpu.m_registers[1], value);
            ASSERT_EQ(cpu.m_conditionalCodes.p, 1);
        }

      protected:
        CPU cpu;
};

TEST_F(CPUTests, AddInstruction)
{
    testAddInstruction();
}

TEST_F(CPUTests, LdInstruction)
{
    testLoadInstruction();
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}