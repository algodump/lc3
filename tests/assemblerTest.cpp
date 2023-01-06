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

TEST(Instructions, BrInstruction)
{
    auto testBrInstructionWith = [](const std::string& conditionalCodes) {
        std::string label = "LABEL";
        SymbolTable::the().add(label, 1);

        std::string opCode = "0000";

        std::string conditionalCodesResult = "";
        if (conditionalCodes.empty() || conditionalCodes.size() == 3) {
            conditionalCodesResult += "111";
        } else {
            char n =
                conditionalCodes.find('n') != std::string::npos ? '1' : '0';
            char z =
                conditionalCodes.find('z') != std::string::npos ? '1' : '0';
            char p =
                conditionalCodes.find('p') != std::string::npos ? '1' : '0';
            conditionalCodesResult += n;
            conditionalCodesResult += z;
            conditionalCodesResult += p;
        }

        std::string labelOffset =
            std::bitset<9>(SymbolTable::the().get(label)).to_string();

        std::string expectedResult =
            opCode + conditionalCodesResult + labelOffset;

        BrInstruction brInstruction(conditionalCodes, label);
        std::bitset<16> binaryLoadInstruction(brInstruction.generate());

        ASSERT_EQ(binaryLoadInstruction.to_string(), expectedResult);
    };

    testBrInstructionWith("");
    testBrInstructionWith("n");
    testBrInstructionWith("zp");
    testBrInstructionWith("z");
    testBrInstructionWith("np");
    testBrInstructionWith("p");
    testBrInstructionWith("nz");
    testBrInstructionWith("nzp");
}

TEST(Instructions, JmpAndRetInsturctions)
{
    // RET is special case of jump, so it's covered here
    std::vector<uint8_t> lc3registers {0, 1, 2, 3, 4, 5, 6, 7};
    for (auto lc3Register : lc3registers) {
        JmpInsturction jmpIntruction(lc3Register);
        std::string opcode = "1100";
        std::string lc3RegisterBin = std::bitset<3>(lc3Register).to_string();
        std::string expectedResult = opcode + "000" + lc3RegisterBin + "000000";

        std::bitset<16> binaryJumpInstruction(jmpIntruction.generate());
        ASSERT_EQ(binaryJumpInstruction.to_string(), expectedResult);
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}