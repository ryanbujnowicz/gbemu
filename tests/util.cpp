#include <gtest/gtest.h>

#include "util.h"

TEST(UtilTest, ToStr)
{
    EXPECT_EQ("0x00", gb::toStr(static_cast<gb::Word>(0x00)));
    EXPECT_EQ("0x0F", gb::toStr(static_cast<gb::Word>(0x0F)));
    EXPECT_EQ("0x10", gb::toStr(static_cast<gb::Word>(0x10)));
    EXPECT_EQ("0xFF", gb::toStr(static_cast<gb::Word>(0xFF)));
    EXPECT_EQ("0xFF", gb::toStr(static_cast<gb::Word>(0xFF)));
    EXPECT_EQ("0x0000", gb::toStr(static_cast<gb::Dword>(0x0000)));
    EXPECT_EQ("0x00F0", gb::toStr(static_cast<gb::Dword>(0x00F0)));
    EXPECT_EQ("0x0010", gb::toStr(static_cast<gb::Dword>(0x0010)));
    EXPECT_EQ("0xFF00", gb::toStr(static_cast<gb::Dword>(0xFF00)));
    EXPECT_EQ("0x00FF", gb::toStr(static_cast<gb::Dword>(0x00FF)));
    EXPECT_EQ("0xFFFF", gb::toStr(static_cast<gb::Dword>(0xFFFF)));
}

TEST(UtilTest, ToHex)
{
    EXPECT_EQ(0x00, gb::toHex<gb::Word>("0x00"));
    EXPECT_EQ(0x0F, gb::toHex<gb::Word>("0x0F"));
    EXPECT_EQ(0x10, gb::toHex<gb::Word>("0x10"));
    EXPECT_EQ(0xF0, gb::toHex<gb::Word>("0xF0"));
    EXPECT_EQ(0xFF, gb::toHex<gb::Word>("0xFF"));
    EXPECT_EQ(0x0000, gb::toHex<gb::Dword>("0x0000"));
    EXPECT_EQ(0x000F, gb::toHex<gb::Dword>("0x000F"));
    EXPECT_EQ(0x0010, gb::toHex<gb::Dword>("0x0010"));
    EXPECT_EQ(0x00FF, gb::toHex<gb::Dword>("0x00FF"));
    EXPECT_EQ(0xFF00, gb::toHex<gb::Dword>("0xFF00"));
    EXPECT_EQ(0xFFFF, gb::toHex<gb::Dword>("0xFFFF"));
}

