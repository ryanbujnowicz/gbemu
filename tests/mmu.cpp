#include <gtest/gtest.h>

#include "cpu/mmu.h"
#include "cpu/memory.h"
#include "util/units.h"

class MMUTest : public testing::Test
{
protected:

    MMUTest() :
        _mem1(32),
        _mem2(16)
    {
        _mmu.map(&_mem1, gb::Range(0x10, 0x1F), gb::Range(0x00, 0x0F));
        _mmu.map(&_mem1, gb::Range(0x00, 0x0F), gb::Range(0x10, 0x1F));
        _mmu.map(&_mem2, gb::Range(0x00, 0x0F), gb::Range(0x100, 0x10F));
    }

    gb::MMU _mmu;
    gb::Memory _mem1;
    gb::Memory _mem2;
};

TEST_F(MMUTest, SetGet)
{
    _mmu[0x00] = 5;
    _mmu[0x01] = 6;
    _mmu[0x15] = 100;
    _mmu[0x100] = 1;
    _mmu[0x103] = 3;
    EXPECT_EQ(5, _mem1[0x10]);
    EXPECT_EQ(6, _mem1[0x11]);
    EXPECT_EQ(100, _mem1[0x05]);
    EXPECT_EQ(1, _mem2[0x00]);
    EXPECT_EQ(3, _mem2[0x03]);
}

TEST_F(MMUTest, ValidAddress)
{
    EXPECT_TRUE(_mmu.isValidAddress(0x00));
    EXPECT_TRUE(_mmu.isValidAddress(0x0F));
    EXPECT_TRUE(_mmu.isValidAddress(0x02));
    EXPECT_TRUE(_mmu.isValidAddress(0x10));
    EXPECT_TRUE(_mmu.isValidAddress(0x1F));
    EXPECT_TRUE(_mmu.isValidAddress(0x100));
    EXPECT_TRUE(_mmu.isValidAddress(0x10F));
    EXPECT_FALSE(_mmu.isValidAddress(0x20));
    EXPECT_FALSE(_mmu.isValidAddress(0xFF));
    EXPECT_FALSE(_mmu.isValidAddress(0x110));
}

