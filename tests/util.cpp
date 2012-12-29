#include <gtest/gtest.h>

#include "util/util.h"

TEST(UtilTest, ToStr)
{
    EXPECT_EQ("0x00", gb::toStr(static_cast<gb::Byte>(0x00)));
    EXPECT_EQ("0x0F", gb::toStr(static_cast<gb::Byte>(0x0F)));
    EXPECT_EQ("0x10", gb::toStr(static_cast<gb::Byte>(0x10)));
    EXPECT_EQ("0xFF", gb::toStr(static_cast<gb::Byte>(0xFF)));
    EXPECT_EQ("0xFF", gb::toStr(static_cast<gb::Byte>(0xFF)));
    EXPECT_EQ("0x0000", gb::toStr(static_cast<gb::Word>(0x0000)));
    EXPECT_EQ("0x00F0", gb::toStr(static_cast<gb::Word>(0x00F0)));
    EXPECT_EQ("0x0010", gb::toStr(static_cast<gb::Word>(0x0010)));
    EXPECT_EQ("0xFF00", gb::toStr(static_cast<gb::Word>(0xFF00)));
    EXPECT_EQ("0x00FF", gb::toStr(static_cast<gb::Word>(0x00FF)));
    EXPECT_EQ("0xFFFF", gb::toStr(static_cast<gb::Word>(0xFFFF)));
}

TEST(UtilTest, ToHex)
{
    EXPECT_EQ(0x00, gb::toHex<gb::Byte>("0x00"));
    EXPECT_EQ(0x0F, gb::toHex<gb::Byte>("0x0F"));
    EXPECT_EQ(0x10, gb::toHex<gb::Byte>("0x10"));
    EXPECT_EQ(0xF0, gb::toHex<gb::Byte>("0xF0"));
    EXPECT_EQ(0xFF, gb::toHex<gb::Byte>("0xFF"));
    EXPECT_EQ(0x0000, gb::toHex<gb::Word>("0x0000"));
    EXPECT_EQ(0x000F, gb::toHex<gb::Word>("0x000F"));
    EXPECT_EQ(0x0010, gb::toHex<gb::Word>("0x0010"));
    EXPECT_EQ(0x00FF, gb::toHex<gb::Word>("0x00FF"));
    EXPECT_EQ(0xFF00, gb::toHex<gb::Word>("0xFF00"));
    EXPECT_EQ(0xFFFF, gb::toHex<gb::Word>("0xFFFF"));
}

TEST(UtilTest, ToSigned)
{
    EXPECT_EQ(0x00, gb::toSigned8(0));
    EXPECT_EQ(0x05, gb::toSigned8(5));
    EXPECT_EQ(0xFB, gb::toSigned8(-5));
    EXPECT_EQ(0x64, gb::toSigned8(100));
    EXPECT_EQ(0x9C, gb::toSigned8(-100));
    EXPECT_EQ(0x0000, gb::toSigned16(0));
    EXPECT_EQ(0x0005, gb::toSigned16(5));
    EXPECT_EQ(0xFFFB, gb::toSigned16(-5));
    EXPECT_EQ(0x0064, gb::toSigned16(100));
    EXPECT_EQ(0xFF9C, gb::toSigned16(-100));
}

TEST(UtilTest, ToInt)
{
    EXPECT_EQ(0, gb::toInt8(0x00));
    EXPECT_EQ(5, gb::toInt8(0x05));
    EXPECT_EQ(-5, gb::toInt8(0xFB));
    EXPECT_EQ(100, gb::toInt8(0x64));
    EXPECT_EQ(-100, gb::toInt8(0x9C));
    EXPECT_EQ(0, gb::toInt16(0x0000));
    EXPECT_EQ(5, gb::toInt16(0x0005));
    EXPECT_EQ(-5, gb::toInt16(0xFFFB));
    EXPECT_EQ(100, gb::toInt16(0x0064));
    EXPECT_EQ(-100, gb::toInt16(0xFF9C));
}

TEST(UtilTest, RangeMinMax)
{
    EXPECT_EQ(5, gb::Range(5, 6).min());
    EXPECT_EQ(6, gb::Range(5, 6).max());
    EXPECT_EQ(1, gb::Range(1, 10).min());
    EXPECT_EQ(16, gb::Range(14, 16).max());
}


TEST(UtilTest, RangeContains)
{
    EXPECT_TRUE(gb::Range(5, 6).contains(5));
    EXPECT_TRUE(gb::Range(5, 6).contains(6));
    EXPECT_FALSE(gb::Range(5, 6).contains(4));
    EXPECT_FALSE(gb::Range(5, 6).contains(7));
}

