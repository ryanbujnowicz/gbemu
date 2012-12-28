#include <gtest/gtest.h>

#include "cpu/memory.h"
#include "cpu/units.h"

class MemoryTest : public testing::Test
{
protected:

    MemoryTest() :
        _mem(gb::bytes(16))
    {
    }

    gb::Memory _mem;
};

TEST_F(MemoryTest, Size)
{
    EXPECT_EQ(gb::bytes(16), _mem.size());
}

TEST_F(MemoryTest, SetGet)
{
    _mem[0x04] = 85;  
    _mem[0x05] = 112;  
    _mem[0x06] = 142;  
    EXPECT_EQ(85, _mem[0x04]);
    EXPECT_EQ(112, _mem[0x05]);
    EXPECT_EQ(142, _mem[0x06]);
}

TEST_F(MemoryTest, ValidAddress)
{
    EXPECT_TRUE(_mem.isValidAddress(0x00));
    EXPECT_TRUE(_mem.isValidAddress(0x05));
    EXPECT_TRUE(_mem.isValidAddress(0x0F));
    EXPECT_FALSE(_mem.isValidAddress(0x10));
    EXPECT_FALSE(_mem.isValidAddress(0xFF));
}

