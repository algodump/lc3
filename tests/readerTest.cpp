#include <gtest/gtest.h>

#include "../reader.hpp"

template <class T> bool is(const std::shared_ptr<Instruction>& insruction)
{
    return dynamic_cast<T*>(insruction.get()) != nullptr;
}

bool isOriginAndEndPresent(
    const std::vector<std::shared_ptr<Instruction>>& instructions)
{
    return is<OriginDerective>(instructions.front()) &&
           is<EndDerective>(instructions.back());
}

TEST(Reader, TestAssemlbyDirectives)
{
    Reader r("..\\..\\..\\tests\\testDirectives.lc3");

    auto parsedInstructions = r.readFile();

    ASSERT_EQ(parsedInstructions.size(), 5);
    EXPECT_TRUE(isOriginAndEndPresent(parsedInstructions));

    EXPECT_TRUE(is<StringDerective>(parsedInstructions[1]));
    EXPECT_TRUE(is<BlkwDerective>(parsedInstructions[2]));
    EXPECT_TRUE(is<FillDerective>(parsedInstructions[3]));
}

TEST(Reader, TestInstructions)
{
    // FIXME: fix pathes
    Reader r("..\\..\\..\\tests\\testInstructions.lc3");

    auto parsedInstructions = r.readFile();

    ASSERT_EQ(parsedInstructions.size(), 4);

    EXPECT_TRUE(isOriginAndEndPresent(parsedInstructions));
    EXPECT_TRUE(is<AddInstruction>(parsedInstructions[1]));
    EXPECT_TRUE(is<LoadInstruction>(parsedInstructions[2]));
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}