#include <gtest/gtest.h>

#include "util.h"

TEST(UtilTest, ToStr)
{
    EXPECT_EQ("0x00", gb::toStr(0x00));
    EXPECT_EQ("0x0F", gb::toStr(0x0F));
    EXPECT_EQ("0x10", gb::toStr(0x10));
    EXPECT_EQ("0xFF", gb::toStr(0xFF));
    EXPECT_EQ("0xFF", gb::toStr(0xFF));
}

TEST(UtilTest, ToHex)
{
    EXPECT_EQ(0x00, gb::toHex("0x00"));
    EXPECT_EQ(0x0F, gb::toHex("0x0F"));
    EXPECT_EQ(0x10, gb::toHex("0x10"));
    EXPECT_EQ(0xFF, gb::toHex("0xFF"));
    EXPECT_EQ(0xFF, gb::toHex("0xFF"));
}

