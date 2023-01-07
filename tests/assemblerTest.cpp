#include <gtest/gtest.h>

#include "../assembler.hpp"
#include "../reader.hpp"


namespace {
template <class InstructionType>
void testAllTheRegisterFor(const std::string& offsetOrLabel)
{
    std::vector<uint8_t> lc3registers{0, 1, 2, 3, 4, 5, 6, 7};
    for (auto lc3Register : lc3registers) {
        InstructionType instruction(lc3Register, offsetOrLabel);
        std::bitset<16> binaryInstruction(instruction.generate());

        std::string offset = Assembler::getBinaryOffsetToJumpTo<9>(offsetOrLabel);
        std::string lc3RegisterBin = Assembler::toBinaryString<3>(lc3Register);
        std::string expectedResult =
            instruction.opcode() + lc3RegisterBin + offset;
        ASSERT_EQ(binaryInstruction.to_string(), expectedResult);
    }
}

template<class InstructionType>
void testForDestinationSource1Source2OrImmediate()
{
    uint8_t destinationRegister = 0;
    uint8_t source1Register = 1;
    uint8_t source2Register = 2;
    InstructionType addInstructions(destinationRegister, source1Register,
                                   source2Register, false);
    std::bitset<16> binaryAddInstruction(addInstructions.generate());

    std::string expectedResult =
        addInstructions.opcode() +
        Assembler::toBinaryString<3>(destinationRegister) +
        Assembler::toBinaryString<3>(source1Register) + "000" +
        Assembler::toBinaryString<3>(source2Register);
    ASSERT_EQ(binaryAddInstruction.to_string(), expectedResult);

    uint8_t immediateValue = 16;
    InstructionType addInstructionsWithImmediat(
        destinationRegister, source1Register, immediateValue, true);
    std::bitset<16> binaryAddInstructionWithImmediate(
        addInstructionsWithImmediat.generate());

    std::string expectedResultWithImmediate =
        addInstructionsWithImmediat.opcode() +
        Assembler::toBinaryString<3>(destinationRegister) +
        Assembler::toBinaryString<3>(source1Register) + "1" +
        Assembler::toBinaryString<5>(immediateValue);
    ASSERT_EQ(binaryAddInstructionWithImmediate.to_string(),
              expectedResultWithImmediate);
}
} // namespace

TEST(Instructions, AddInstruction)
{
    testForDestinationSource1Source2OrImmediate<AddInstruction>();
}

TEST(Instructions, AndInstruction)
{
    testForDestinationSource1Source2OrImmediate<AddInstruction>();
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
        std::bitset<16> binaryLdInstruction(brInstruction.generate());

        std::string labelOffset =
            Assembler::toBinaryString(SymbolTable::the().get(label));
        std::string expectedResult =
            brInstruction.opcode() + conditionalCodesResult + labelOffset;
        ASSERT_EQ(binaryLdInstruction.to_string(), expectedResult);
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
    auto testJsr = [](const std::string& labelOrOffset) {
        JsrInstruction jsrInstruction(labelOrOffset);
        std::bitset<16> binaryJsrInstruction(jsrInstruction.generate());
        std::string offset = Assembler::getBinaryOffsetToJumpTo<11>(labelOrOffset);
        std::string expectedResult =
            jsrInstruction.opcode() + "1" + offset;
        ASSERT_EQ(binaryJsrInstruction.to_string(), expectedResult);
    };

    std::string label = "LABEL";
    SymbolTable::the().add(label, 4);
    testJsr(label);

    std::string offset = "#3131";
    testJsr(offset);
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

TEST(Instructions, LdInstruction)
{
    std::string label = "LABEL";
    SymbolTable::the().add(label, 1);
    testAllTheRegisterFor<LdInstruction>(label);

    std::string offset = "#3200";
    testAllTheRegisterFor<LdInstruction>(offset);
}

TEST(Instructions, LdiInsturction)
{
    std::string label = "WOW";
    SymbolTable::the().add(label, 42);
    testAllTheRegisterFor<LdiInsturction>(label);

    std::string offset = "#6666";
    testAllTheRegisterFor<LdiInsturction>(offset);
}

TEST(Instructions, LeaInstruction)
{
    std::string label = "lea";
    SymbolTable::the().add(label, 24);
    testAllTheRegisterFor<LeaInstruction>(label);

    std::string offset = "#3333";
    testAllTheRegisterFor<LeaInstruction>(offset);
}

TEST(Instructions, StInstruction)
{
    std::string label = "st";
    SymbolTable::the().add(label, 11);
    testAllTheRegisterFor<StInstruction>(label);

    std::string offset = "#4444";
    testAllTheRegisterFor<StInstruction>(offset);
}

TEST(Instructions, StiInstruction)
{
    std::string label = "sti";
    SymbolTable::the().add(label, 9);
    testAllTheRegisterFor<StiInstruction>(label);

    std::string offset = "#5555";
    testAllTheRegisterFor<StiInstruction>(offset);
}

TEST(Instructions, LdrInstruction)
{
    std::vector<uint8_t> lc3registers{0, 1, 2, 3, 4, 5, 6, 7};
    for (auto lc3BaseRegister : lc3registers) {
        std::string offset = "#5";
        uint8_t lc3DestinationRegister =
            (lc3BaseRegister + 1) % lc3registers.size();
        LdrInstruction ldrInstruction(lc3DestinationRegister,
                                      lc3BaseRegister,
                                      offset);
        std::bitset<16> binaryLdrInstruction(ldrInstruction.generate());

        std::string lc3BaseRegisterBin =
            Assembler::toBinaryString<3>(lc3BaseRegister);
        std::string lc3DestinationRegisterBin =
            Assembler::toBinaryString<3>(lc3DestinationRegister);
        std::string binaryOffset = Assembler::getBinaryOffsetToJumpTo<6>(offset);

        std::string expectedResult = ldrInstruction.opcode() +
                                     lc3DestinationRegisterBin +
                                     lc3BaseRegisterBin + binaryOffset;
        ASSERT_EQ(binaryLdrInstruction.to_string(), expectedResult);
    }
}

TEST(Instructions, NotInstruction)
{
    std::vector<uint8_t> lc3registers{0, 1, 2, 3, 4, 5, 6, 7};
    for (auto lc3sourceRegister : lc3registers) {
        uint8_t lc3DestinationRegister =
            (lc3sourceRegister + 1) % lc3registers.size();
        NotInstruction ldrInstruction(lc3DestinationRegister,
                                      lc3sourceRegister);
        std::bitset<16> binaryLdrInstruction(ldrInstruction.generate());

        std::string lc3BaseRegisterBin =
            Assembler::toBinaryString<3>(lc3sourceRegister);
        std::string lc3DestinationRegisterBin =
            Assembler::toBinaryString<3>(lc3DestinationRegister);

        std::string expectedResult = ldrInstruction.opcode() +
                                     lc3DestinationRegisterBin +
                                     lc3BaseRegisterBin + "111111";
        ASSERT_EQ(binaryLdrInstruction.to_string(), expectedResult);
    }
}

TEST(Instructions, RtiInstruction)
{
    RtiInstruction rtiInstruction;
    ASSERT_EQ(Assembler::toBinaryString<16>(rtiInstruction.generate()),
              "1000000000000000");
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}