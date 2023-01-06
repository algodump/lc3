#include <gtest/gtest.h>

#include "../reader.hpp"
#include "../assembler.hpp"

TEST(Instructions, AddInstruction)
{
    std::vector<std::string> operands = {"R0", "R1", "R2"};
    AddInstruction addInstructions(operands);
    std::bitset<16> binaryAddInstruction(addInstructions.generate());

    // NOTE: ideally retrive this binary register numbers form operands
    std::string expectedResult =
        addInstructions.opcode() + "000" + "001" + "000" + "010";
    ASSERT_EQ(binaryAddInstruction.to_string(), expectedResult);

    std::vector<std::string> operandsWithImmediate = {"R0", "R1", "7"};
    AddInstruction addInstructionsWithImmediat(operandsWithImmediate);
    std::bitset<16> binaryAddInstructionWithImmediate(
        addInstructionsWithImmediat.generate());

    std::string expectedResultWithImmediate =
        addInstructionsWithImmediat.opcode() + "000" + "001" + "1" + "00111";
    ASSERT_EQ(binaryAddInstructionWithImmediate.to_string(),
              expectedResultWithImmediate);
}

TEST(Instructions, LoadInstruction)
{
    std::string label = "LABEL";
    SymbolTable::the().add(label, 1);
    std::vector<std::string> operands = {"R2", label};
    LoadInstruction loadInstruction(operands);
    std::bitset<16> binaryLoadInstruction(loadInstruction.generate());

    std::string labelOffset =
        Assembler::toBinaryString(SymbolTable::the().get(label));
    std::string expectedResult = loadInstruction.opcode() + "010" + labelOffset;
    ASSERT_EQ(binaryLoadInstruction.to_string(), expectedResult);
}

TEST(Instructions, AndInstruction)
{
    std::vector<std::string> operands = {"R7", "R1", "R2"};
    AndInstruction andInstruction(operands);
    std::bitset<16> binaryAddInstruction(andInstruction.generate());

    std::string expectedResult =
        andInstruction.opcode() + "111" + "001" + "000" + "010";
    ASSERT_EQ(binaryAddInstruction.to_string(), expectedResult);

    std::vector<std::string> operandsWithImmediate = {"R7", "R1", "16"};
    AndInstruction andInstructionsWithImmediat(operandsWithImmediate);
    std::bitset<16> binaryAndInstructionWithImmediate(
        andInstructionsWithImmediat.generate());

    std::string expectedResultWithImmediate =
        andInstructionsWithImmediat.opcode() + "111" + "001" + "1" + "10000";
    ASSERT_EQ(binaryAndInstructionWithImmediate.to_string(),
              expectedResultWithImmediate);
}

TEST(Instructions, BrInstruction)
{
    auto testBrInstructionWith = [](const std::string& conditionalCodes) {
        std::string label = "LABEL";
        SymbolTable::the().add(label, 1);

        std::string conditionalCodesResult = "";
        if (conditionalCodes.empty() || conditionalCodes.size() == 3) {
            conditionalCodesResult += "111";
        }
        else {
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

        BrInstruction brInstruction(conditionalCodes, label);
        std::bitset<16> binaryLoadInstruction(brInstruction.generate());

        std::string labelOffset = Assembler::toBinaryString(SymbolTable::the().get(label));
        std::string expectedResult =
            brInstruction.opcode() + conditionalCodesResult + labelOffset;
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
    std::vector<uint8_t> lc3registers{0, 1, 2, 3, 4, 5, 6, 7};
    for (auto lc3Register : lc3registers) {
        JmpInsturction jmpIntruction(lc3Register);
        std::bitset<16> binaryJumpInstruction(jmpIntruction.generate());

        std::string lc3RegisterBin = Assembler::toBinaryString<3>(lc3Register);
        std::string expectedResult =
            jmpIntruction.opcode() + "000" + lc3RegisterBin + "000000";
        ASSERT_EQ(binaryJumpInstruction.to_string(), expectedResult);
    }
}

TEST(Instructions, JsrInstruction)
{
    std::string label = "LABEL";
    SymbolTable::the().add(label, 4);

    JsrInstruction jsrInstruction(label);
    std::bitset<16> binaryJsrInstruction(jsrInstruction.generate());

    std::string labelOffset = Assembler::toBinaryString<11>(SymbolTable::the().get(label));
    std::string expectedResult = jsrInstruction.opcode() + "1" + labelOffset;
    ASSERT_EQ(binaryJsrInstruction.to_string(), expectedResult);
}

TEST(Instructions, JsrrInstruction)
{
    std::vector<uint8_t> lc3registers{0, 1, 2, 3, 4, 5, 6, 7};
    for (auto lc3Register : lc3registers) {
        JsrrInstruction jsrInstruction(lc3Register);
        std::bitset<16> binaryJsrInstruction(jsrInstruction.generate());

        std::string lc3RegisterBin = Assembler::toBinaryString<3>(lc3Register);
        std::string expectedResult =
            jsrInstruction.opcode() + "0" + "00" + lc3RegisterBin + "000000";
        ASSERT_EQ(binaryJsrInstruction.to_string(), expectedResult);
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}