#include <gtest/gtest.h>

#include "../reader.hpp"

TEST(Instructions, AddInstruction)
{
    std::vector<std::string> operands = {"R0", "R1", "R2"};

    AddInstruction addInstructions(operands);
    std::bitset<16> binaryAddInstruction(addInstructions.generate());
    ASSERT_EQ(binaryAddInstruction.to_string(), "0001000001000010");

    std::vector<std::string> operandsWithImmediate = {"R0", "R1", "7"};

    AddInstruction addInstructionsWithImmediat(operandsWithImmediate);
    std::bitset<16> binaryAddInstructionWithImmediate(
        addInstructionsWithImmediat.generate());
    ASSERT_EQ(binaryAddInstructionWithImmediate.to_string(),
              "0001000001100111");
}

TEST(Instructions, LoadInstruction)
{
    std::string label = "LABEL";
    SymbolTable::the().add(label, 1);
    std::vector<std::string> operands = {"R2", label};

    LoadInstruction loadInstruction(operands);
    std::bitset<16> binaryLoadInstruction(loadInstruction.generate());
    ASSERT_EQ(binaryLoadInstruction.to_string(), "0010010000000001");
}

TEST(Instructions, AndInstruction)
{
    std::vector<std::string> operands = {"R7", "R1", "R2"};

    AndInstruction andInstruction(operands);
    std::bitset<16> binaryAddInstruction(andInstruction.generate());
    ASSERT_EQ(binaryAddInstruction.to_string(), "0101111001000010");

    std::vector<std::string> operandsWithImmediate = {"R7", "R1", "16"};

    AndInstruction andInstructionsWithImmediat(operandsWithImmediate);
    std::bitset<16> binaryAndInstructionWithImmediate(
        andInstructionsWithImmediat.generate());
    ASSERT_EQ(binaryAndInstructionWithImmediate.to_string(),
              "0101111001110000");
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}