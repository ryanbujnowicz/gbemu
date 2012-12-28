#include <gtest/gtest.h>

#include "cpu/cpu.h"
#include "cpu/memory.h"
#include "cpu/units.h"
#include "cpu/util.h"

#define EXPECT_FLAGS(z, n, h, c) \
    EXPECT_EQ(z, _cpu.flag(gb::Cpu::FlagZ)); \
    EXPECT_EQ(n, _cpu.flag(gb::Cpu::FlagN)); \
    EXPECT_EQ(h, _cpu.flag(gb::Cpu::FlagH)); \
    EXPECT_EQ(c, _cpu.flag(gb::Cpu::FlagC)); \

class CpuTest : public testing::Test
{
protected:

    const size_t MemSize = 100000;;

    CpuTest() :
        _mem(MemSize)
    {
        _cpu.setMemory(&_mem);
        for (size_t i = 0; i < MemSize; ++i) {
            _mem[i] = i;
        }

        _cpu.registers().A  = 0x07;
        _cpu.registers().F  = 0x00;
        _cpu.registers().BC = 0xF00D;
        _cpu.registers().DE = 0x1023;
        _cpu.registers().HL = 0xF0F0;
        _cpu.registers().SP = 0x0010;
        _cpu.registers().PC = 0x0000;

        _cpu.setInterruptsEnabled(false);
    }

protected:
    gb::Cpu _cpu;
    gb::Memory _mem;

    void _loadAndExecute(gb::Word opcode)
    {
        _mem[_cpu.registers().PC] = opcode; 
        _cpu.processNextInstruction();
    }

    void _loadAndExecute(gb::Word opcode, gb::Word arg)
    {
        _mem[_cpu.registers().PC] = opcode; 
        _mem[_cpu.registers().PC + 0x0001] = arg; 
        _cpu.processNextInstruction();
    }

    void _loadAndExecute(gb::Word opcode, gb::Word arg1, gb::Word arg2)
    {
        _mem[_cpu.registers().PC] = opcode; 
        _mem[_cpu.registers().PC + 0x0001] = arg1; 
        _mem[_cpu.registers().PC + 0x0002] = arg2; 
        _cpu.processNextInstruction();
    }
};

TEST_F(CpuTest, PCIncTest)
{
    // Instruction with no args
    _loadAndExecute(0x00);
    EXPECT_EQ(0x01, _cpu.registers().PC);

    // Instruction with 1 args
    _cpu.registers().PC = 0x0000;
    _loadAndExecute(0x06, 0x00);
    EXPECT_EQ(0x02, _cpu.registers().PC);

    // Instruction with 2 args
    _cpu.registers().PC = 0x0000;
    _loadAndExecute(0x01, 0x00, 0x00);
    EXPECT_EQ(0x03, _cpu.registers().PC);
}

TEST_F(CpuTest, Opcode0x00Test)
{
    // NOP
    _loadAndExecute(0x00);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x01Test)
{
    // LD BC,nn
    _loadAndExecute(0x01, 0x10, 0xFC);
    EXPECT_EQ(0xFC, _cpu.registers().B);
    EXPECT_EQ(0x10, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x02Test)
{
    // LD (BC),A
    _loadAndExecute(0x02);
    EXPECT_EQ(0x07, _mem[_cpu.registers().BC]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x03Test)
{
    // INC BC
    gb::Dword prev = _cpu.registers().BC;

    _loadAndExecute(0x03);
    EXPECT_EQ(prev + 1, _cpu.registers().BC);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().BC = 0xFFFF;
    prev = _cpu.registers().BC;

    _loadAndExecute(0x03);
    EXPECT_EQ(0x00, _cpu.registers().BC);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x04Test)
{
    // INC B
    gb::Word prev = _cpu.registers().B;

    _loadAndExecute(0x04);
    EXPECT_EQ(prev + 1, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().B = 0x8F;
    prev = _cpu.registers().B;

    _loadAndExecute(0x04);
    EXPECT_EQ(prev + 1, _cpu.registers().B);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().B = 0xFF;
    prev = _cpu.registers().B;

    _loadAndExecute(0x04);
    EXPECT_EQ(0x00, _cpu.registers().B);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0x05Test)
{
    // DEC B
    _cpu.registers().B = 0x05;
    _loadAndExecute(0x05);
    EXPECT_EQ(0x04, _cpu.registers().B);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().B = 0x01;
    _loadAndExecute(0x05);
    EXPECT_EQ(0x00, _cpu.registers().B);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().B = 0x10;
    _loadAndExecute(0x05);
    EXPECT_EQ(0x0F, _cpu.registers().B);
    EXPECT_FLAGS(0,1,1,0);
}

TEST_F(CpuTest, Opcode0x06Test)
{
    // LD B,n
    _loadAndExecute(0x06, 0xF0);
    EXPECT_EQ(0xF0, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x07Test)
{
    // RLCA
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0x07);
    EXPECT_EQ(19, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().A = 9; // 00001001
    _loadAndExecute(0x07);
    EXPECT_EQ(18, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x08Test)
{
    // LD (nn),SP
    _loadAndExecute(0x08, 0x10, 0xF0);
    EXPECT_EQ(_cpu.registers().SP, _mem[0xF010]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x09Test)
{
    // ADD HL,BC
    _cpu.registers().HL = 0x0F0F;
    _cpu.registers().BC = 0x00F1;
    _loadAndExecute(0x09);
    EXPECT_EQ(0x1000, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().HL = 0x0001;
    _cpu.registers().BC = 0x0001;
    _loadAndExecute(0x09);
    EXPECT_EQ(0x0002, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().HL = 0xFFFF;
    _cpu.registers().BC = 0x0001;
    _loadAndExecute(0x09);
    EXPECT_EQ(0x0000, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,1,1);
}

TEST_F(CpuTest, Opcode0x10Test)
{
    // STOP
    EXPECT_FALSE(_cpu.isStopped());
    _loadAndExecute(0x10);
    EXPECT_TRUE(_cpu.isStopped());
}

TEST_F(CpuTest, Opcode0x0ATest)
{
    // LD A,(BC)
    _loadAndExecute(0x0A);
    EXPECT_EQ(_mem[_cpu.registers().BC], _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x0BTest)
{
    // DEC BC
    _cpu.registers().BC = 0x0F00;
    _loadAndExecute(0x0B);
    EXPECT_EQ(0x0EFF, _cpu.registers().BC);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x0CTest)
{
    // INC C
    gb::Word prev = _cpu.registers().C;

    _loadAndExecute(0x0C);
    EXPECT_EQ(prev + 1, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().C = 0x8F;
    prev = _cpu.registers().C;

    _loadAndExecute(0x0C);
    EXPECT_EQ(prev + 1, _cpu.registers().C);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().C = 0xFF;
    prev = _cpu.registers().C;

    _loadAndExecute(0x0C);
    EXPECT_EQ(0x00, _cpu.registers().C);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0x0DTest)
{
    // DEC C
    _cpu.registers().C = 0x05;
    _loadAndExecute(0x0D);
    EXPECT_EQ(0x04, _cpu.registers().C);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().C = 0x01;
    _loadAndExecute(0x0D);
    EXPECT_EQ(0x00, _cpu.registers().C);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().C = 0x10;
    _loadAndExecute(0x0D);
    EXPECT_EQ(0x0F, _cpu.registers().C);
    EXPECT_FLAGS(0,1,1,0);
}

TEST_F(CpuTest, Opcode0x0ETest)
{
    // LD C,n
    _loadAndExecute(0x0E, 0xF7);
    EXPECT_EQ(0xF7, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x0FTest)
{
    // RRCA
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0x0F);
    EXPECT_EQ(196, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().A = 136; // 10001000
    _loadAndExecute(0x0F);
    EXPECT_EQ(68, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x11Test)
{
    // LD DE,nn
    _loadAndExecute(0x11, 0x89, 0xF7);
    EXPECT_EQ(0xF789, _cpu.registers().DE);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x12Test)
{
    // LD (DE),A
    _loadAndExecute(0x12);
    EXPECT_EQ(_cpu.registers().A, _mem[_cpu.registers().DE]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x13Test)
{
    // INC DE
    gb::Dword prev = _cpu.registers().DE;

    _loadAndExecute(0x13);
    EXPECT_EQ(prev + 1, _cpu.registers().DE);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().DE = 0xFFFF;
    prev = _cpu.registers().DE;

    _loadAndExecute(0x13);
    EXPECT_EQ(0x00, _cpu.registers().DE);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x14Test)
{
    // INC D
    gb::Word prev = _cpu.registers().D;

    _loadAndExecute(0x14);
    EXPECT_EQ(prev + 1, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().D = 0x8F;
    prev = _cpu.registers().D;

    _loadAndExecute(0x14);
    EXPECT_EQ(prev + 1, _cpu.registers().D);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().D = 0xFF;
    prev = _cpu.registers().D;

    _loadAndExecute(0x14);
    EXPECT_EQ(0x00, _cpu.registers().D);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0x15Test)
{
    // DEC D
    _cpu.registers().D = 0x05;
    _loadAndExecute(0x15);
    EXPECT_EQ(0x04, _cpu.registers().D);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().D = 0x01;
    _loadAndExecute(0x15);
    EXPECT_EQ(0x00, _cpu.registers().D);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().D = 0x10;
    _loadAndExecute(0x15);
    EXPECT_EQ(0x0F, _cpu.registers().D);
    EXPECT_FLAGS(0,1,1,0);
}

TEST_F(CpuTest, Opcode0x16Test)
{
    // LD D,n
    _loadAndExecute(0x16, 0xF7);
    EXPECT_EQ(0xF7, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x17Test)
{
    // RLA
    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0x17);
    EXPECT_EQ(19, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0x17);
    EXPECT_EQ(18, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 9; // 00001001
    _loadAndExecute(0x17);
    EXPECT_EQ(19, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 9; // 00001001
    _loadAndExecute(0x17);
    EXPECT_EQ(18, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x18Test)
{
    // JR (PC+e)
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x18, gb::toSigned8(5));
    EXPECT_EQ(0x1007, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x18, gb::toSigned8(-100));
    EXPECT_EQ(3998, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x19Test)
{
    // ADD HL,DE
    _cpu.registers().HL = 0x0F0F;
    _cpu.registers().DE = 0x00F1;
    _loadAndExecute(0x19);
    EXPECT_EQ(0x1000, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().HL = 0x0001;
    _cpu.registers().DE = 0x0001;
    _loadAndExecute(0x19);
    EXPECT_EQ(0x0002, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().HL = 0xFFFF;
    _cpu.registers().DE = 0x0001;
    _loadAndExecute(0x19);
    EXPECT_EQ(0x0000, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,1,1);
}

TEST_F(CpuTest, Opcode0x1ATest)
{
    // LD A,(DE)
    _loadAndExecute(0x1A);
    EXPECT_EQ(_mem[_cpu.registers().DE], _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x1BTest)
{
    // DEC DE
    _cpu.registers().DE = 0x0F00;
    _loadAndExecute(0x1B);
    EXPECT_EQ(0x0EFF, _cpu.registers().DE);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x1CTest)
{
    // INC E
    gb::Word prev = _cpu.registers().E;

    _loadAndExecute(0x1C);
    EXPECT_EQ(prev + 1, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().E = 0x8F;
    prev = _cpu.registers().E;

    _loadAndExecute(0x1C);
    EXPECT_EQ(prev + 1, _cpu.registers().E);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().E = 0xFF;
    prev = _cpu.registers().E;

    _loadAndExecute(0x1C);
    EXPECT_EQ(0x00, _cpu.registers().E);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0x1DTest)
{
    // DEC E
    _cpu.registers().E = 0x05;
    _loadAndExecute(0x1D);
    EXPECT_EQ(0x04, _cpu.registers().E);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().E = 0x01;
    _loadAndExecute(0x1D);
    EXPECT_EQ(0x00, _cpu.registers().E);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().E = 0x10;
    _loadAndExecute(0x1D);
    EXPECT_EQ(0x0F, _cpu.registers().E);
    EXPECT_FLAGS(0,1,1,0);
}

TEST_F(CpuTest, Opcode0x1ETest)
{
    // LD E,n
    _loadAndExecute(0x1E, 0x34);
    EXPECT_EQ(0x34, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x1FTest)
{
    // RRA
    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0x1F);
    EXPECT_EQ(196, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0x1F);
    EXPECT_EQ(68, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 9; // 00001001
    _loadAndExecute(0x1F);
    EXPECT_EQ(132, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 9; // 00001001
    _loadAndExecute(0x1F);
    EXPECT_EQ(4, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);
}

TEST_F(CpuTest, Opcode0x20Test)
{
    // JR NZ,(PC+e)
    _cpu.registers().F = 0x00;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x20, gb::toSigned8(5));
    EXPECT_EQ(0x1007, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x20, gb::toSigned8(5));
    EXPECT_EQ(0x1002, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x20, gb::toSigned8(-100));
    EXPECT_EQ(3998, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x20, gb::toSigned8(-100));
    EXPECT_EQ(0x1002, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);
}

TEST_F(CpuTest, Opcode0x21Test)
{
    // LD HL,nn
    _loadAndExecute(0x21, 0x89, 0xF7);
    EXPECT_EQ(0xF789, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x22Test)
{
    // LDI (HL),A
    gb::Word val = _cpu.registers().A;
    gb::Dword hl = _cpu.registers().HL;

    _loadAndExecute(0x22);
    EXPECT_EQ(val, _mem[hl]);
    EXPECT_EQ(hl + 1, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x23Test)
{
    // INC HL
    gb::Dword prev = _cpu.registers().HL;

    _loadAndExecute(0x23);
    EXPECT_EQ(prev + 1, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().HL = 0xFFFF;
    prev = _cpu.registers().HL;

    _loadAndExecute(0x23);
    EXPECT_EQ(0x00, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x24Test)
{
    // INC H
    gb::Word prev = _cpu.registers().H;

    _loadAndExecute(0x24);
    EXPECT_EQ(prev + 1, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().H = 0x8F;
    prev = _cpu.registers().H;

    _loadAndExecute(0x24);
    EXPECT_EQ(prev + 1, _cpu.registers().H);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().H = 0xFF;
    prev = _cpu.registers().H;

    _loadAndExecute(0x24);
    EXPECT_EQ(0x00, _cpu.registers().H);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0x25Test)
{
    // DEC H
    _cpu.registers().H = 0x05;
    _loadAndExecute(0x25);
    EXPECT_EQ(0x04, _cpu.registers().H);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().H = 0x01;
    _loadAndExecute(0x25);
    EXPECT_EQ(0x00, _cpu.registers().H);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().H = 0x10;
    _loadAndExecute(0x25);
    EXPECT_EQ(0x0F, _cpu.registers().H);
    EXPECT_FLAGS(0,1,1,0);
}

TEST_F(CpuTest, Opcode0x26Test)
{
    // LD H,n
    _loadAndExecute(0x26, 0x34);
    EXPECT_EQ(0x34, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x27Test)
{
    // DAA
    // Test every row in the DAA table
    _cpu.registers().A = 0x00;
    _cpu.registers().F = 0x00;
    _loadAndExecute(0x27);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);

    _cpu.registers().A = 0x0A;
    _cpu.registers().F = 0x00;
    _loadAndExecute(0x27);
    EXPECT_EQ(0x0A + 0x06, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x03;
    _cpu.registers().F = 0x20;
    _loadAndExecute(0x27);
    EXPECT_EQ(0x03 + 0x06, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0xA0;
    _cpu.registers().F = 0x00;
    _loadAndExecute(0x27);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,1);

    _cpu.registers().A = 0x9A;
    _cpu.registers().F = 0x00;
    _loadAndExecute(0x27);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,1);

    _cpu.registers().A = 0xA0;
    _cpu.registers().F = 0x20;
    _loadAndExecute(0x27);
    EXPECT_EQ(6, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().A = 0x00;
    _cpu.registers().F = 0x10;
    _loadAndExecute(0x27);
    EXPECT_EQ(0x00 + 0x60, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().A = 0x0A;
    _cpu.registers().F = 0x10;
    _loadAndExecute(0x27);
    EXPECT_EQ(0x0A + 0x66, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().A = 0x00;
    _cpu.registers().F = 0x30;
    _loadAndExecute(0x27);
    EXPECT_EQ(0x00 + 0x66, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().A = 0x00;
    _cpu.registers().F = 0x40;
    _loadAndExecute(0x27);
    EXPECT_EQ(0x00 + 0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0x06;
    _cpu.registers().F = 0x60;
    _loadAndExecute(0x27);
    EXPECT_EQ(0, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0x70;
    _cpu.registers().F = 0x50;
    _loadAndExecute(0x27);
    EXPECT_EQ(16, _cpu.registers().A);
    EXPECT_FLAGS(0,1,0,1);

    _cpu.registers().A = 0x66;
    _cpu.registers().F = 0x70;
    _loadAndExecute(0x27);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,1);
}

TEST_F(CpuTest, Opcode0x28Test)
{
    // JR Z,(PC+e)
    _cpu.registers().F = 0xFF;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x28, gb::toSigned8(5));
    EXPECT_EQ(0x1007, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x28, gb::toSigned8(5));
    EXPECT_EQ(0x1002, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x28, gb::toSigned8(-100));
    EXPECT_EQ(3998, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x28, gb::toSigned8(-100));
    EXPECT_EQ(0x1002, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x29Test)
{
    // ADD HL,HL
    _cpu.registers().HL = 0x0FFF;
    _loadAndExecute(0x29);
    EXPECT_EQ(0x0FFF + 0x0FFF, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().HL = 0x0001;
    _loadAndExecute(0x29);
    EXPECT_EQ(0x0002, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().HL = 0xFFFF;
    _loadAndExecute(0x29);
    EXPECT_EQ(0xFFFE, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,1,1);
}

TEST_F(CpuTest, Opcode0x2ATest)
{
    // LDI A,(HL)
    _cpu.registers().HL = 0x00AA;
    _mem[_cpu.registers().HL] = 0x0F;

    _loadAndExecute(0x2A);
    EXPECT_EQ(_mem[0x00AA], _cpu.registers().A);
    EXPECT_EQ(0x00AA + 1, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x2BTest)
{
    // DEC HL
    _cpu.registers().HL = 0x0F00;
    _loadAndExecute(0x2B);
    EXPECT_EQ(0x0EFF, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x2CTest)
{
    // INC L
    gb::Word prev = _cpu.registers().L;

    _loadAndExecute(0x2C);
    EXPECT_EQ(prev + 1, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().L = 0x8F;
    prev = _cpu.registers().L;

    _loadAndExecute(0x2C);
    EXPECT_EQ(prev + 1, _cpu.registers().L);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().L = 0xFF;
    prev = _cpu.registers().L;

    _loadAndExecute(0x2C);
    EXPECT_EQ(0x00, _cpu.registers().L);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0x2DTest)
{
    // DEC L
    _cpu.registers().L = 0x05;
    _loadAndExecute(0x2D);
    EXPECT_EQ(0x04, _cpu.registers().L);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().L = 0x01;
    _loadAndExecute(0x2D);
    EXPECT_EQ(0x00, _cpu.registers().L);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().L = 0x10;
    _loadAndExecute(0x2D);
    EXPECT_EQ(0x0F, _cpu.registers().L);
    EXPECT_FLAGS(0,1,1,0);
}

TEST_F(CpuTest, Opcode0x2ETest)
{
    // LD L,n
    _loadAndExecute(0x2E, 0x21);
    EXPECT_EQ(0x21, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x2FTest)
{
    // CPL
    _cpu.registers().A = 0xA5;
    _loadAndExecute(0x2F);
    EXPECT_EQ(0x5A, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);
}

TEST_F(CpuTest, Opcode0x30Test)
{
    // JR NC,(PC+e)
    _cpu.registers().F = 0x00;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x30, gb::toSigned8(5));
    EXPECT_EQ(0x1007, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x30, gb::toSigned8(5));
    EXPECT_EQ(0x1002, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x30, gb::toSigned8(-100));
    EXPECT_EQ(3998, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x30, gb::toSigned8(-100));
    EXPECT_EQ(0x1002, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);

}

TEST_F(CpuTest, Opcode0x31Test)
{
    // LD SP,nn
    _loadAndExecute(0x31, 0xFF, 0x27);
    EXPECT_EQ(0x27FF, _cpu.registers().SP);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x32Test)
{
    // LDD (HL),A
    _cpu.registers().HL = 0x00AA;
    _cpu.registers().A = 0x0F;
    _loadAndExecute(0x32);
    EXPECT_EQ(_cpu.registers().A, _mem[0x00AA]);
    EXPECT_EQ(0x00AA - 1, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x33Test)
{
    // INC SP
    gb::Dword prev = _cpu.registers().SP;

    _loadAndExecute(0x33);
    EXPECT_EQ(prev + 1, _cpu.registers().SP);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().SP = 0xFFFF;
    prev = _cpu.registers().SP;

    _loadAndExecute(0x33);
    EXPECT_EQ(0x00, _cpu.registers().SP);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x34Test)
{
    // INC (HL)
    gb::Word prev = _mem[_cpu.registers().HL];

    _loadAndExecute(0x34);
    EXPECT_EQ(prev + 1, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().HL = 0x8F;
    prev = _cpu.registers().HL;

    _loadAndExecute(0x34);
    EXPECT_EQ(prev + 1, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().HL = 0xFF;
    prev = _cpu.registers().HL;

    _loadAndExecute(0x34);
    EXPECT_EQ(0x00, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0x35Test)
{
    // DEC (HL)
    _mem[_cpu.registers().HL] = 0x05;
    _loadAndExecute(0x35);
    EXPECT_EQ(0x04, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,1,0,0);

    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x35);
    EXPECT_EQ(0x00, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(1,1,0,0);

    _mem[_cpu.registers().HL] = 0x10;
    _loadAndExecute(0x35);
    EXPECT_EQ(0x0F, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,1,1,0);
}

TEST_F(CpuTest, Opcode0x36Test)
{
    // LD (HL),n
    _loadAndExecute(0x36, 0x10);
    EXPECT_EQ(0x10, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x37Test)
{
    // SCF
    _cpu.registers().F = 0x00;
    _loadAndExecute(0x37);
    EXPECT_FLAGS(0,0,0,1);
}

TEST_F(CpuTest, Opcode0x38Test)
{
    // JR C,(PC+e)
    _cpu.registers().F = 0xFF;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x38, gb::toSigned8(5));
    EXPECT_EQ(0x1007, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x38, gb::toSigned8(5));
    EXPECT_EQ(0x1002, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x38, gb::toSigned8(-100));
    EXPECT_EQ(3998, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().PC = 0x1000;
    _loadAndExecute(0x38, gb::toSigned8(-100));
    EXPECT_EQ(0x1002, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x39Test)
{
    // ADD HL,SP
    _cpu.registers().HL = 0x0F0F;
    _cpu.registers().SP = 0x00F1;
    _loadAndExecute(0x39);
    EXPECT_EQ(0x1000, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().HL = 0x0001;
    _cpu.registers().SP = 0x0001;
    _loadAndExecute(0x39);
    EXPECT_EQ(0x0002, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().HL = 0xFFFF;
    _cpu.registers().SP = 0x0001;
    _loadAndExecute(0x39);
    EXPECT_EQ(0x0000, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,1,1);
}

TEST_F(CpuTest, Opcode0x3ATest)
{
    // LDD A,(HL)
    _cpu.registers().HL = 0x00AA;
    _mem[_cpu.registers().HL] = 0x0F;
    _loadAndExecute(0x3A);
    EXPECT_EQ(_mem[0x00AA], _cpu.registers().A);
    EXPECT_EQ(0x00AA - 1, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x3BTest)
{
    // DEC SP
    _cpu.registers().SP = 0x0F00;
    _loadAndExecute(0x3B);
    EXPECT_EQ(0x0EFF, _cpu.registers().SP);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x3CTest)
{
    // INC A
    gb::Word prev = _cpu.registers().A;

    _loadAndExecute(0x3C);
    EXPECT_EQ(prev + 1, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x8F;
    prev = _cpu.registers().A;

    _loadAndExecute(0x3C);
    EXPECT_EQ(prev + 1, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xFF;
    prev = _cpu.registers().A;

    _loadAndExecute(0x3C);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0x3DTest)
{
    // DEC A
    _cpu.registers().A = 0x05;
    _loadAndExecute(0x3D);
    EXPECT_EQ(0x04, _cpu.registers().A);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x01;
    _loadAndExecute(0x3D);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0x10;
    _loadAndExecute(0x3D);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);
}

TEST_F(CpuTest, Opcode0x3ETest)
{
    // LD A,n
    _loadAndExecute(0x3E, 0x15);
    EXPECT_EQ(0x15, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x3FTest)
{
    // CCF
    _cpu.registers().F = 0xFF;
    _loadAndExecute(0x3F);
    EXPECT_FLAGS(1,0,0,0);

    _cpu.registers().F = 0x00;
    _loadAndExecute(0x3F);
    EXPECT_FLAGS(0,0,0,1);
}

TEST_F(CpuTest, Opcode0x40Test)
{
    // LD B,B
    _loadAndExecute(0x40);
    EXPECT_EQ(_cpu.registers().B, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x41Test)
{
    // LD B,C
    _loadAndExecute(0x41);
    EXPECT_EQ(_cpu.registers().C, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x42Test)
{
    // LD B,D
    _loadAndExecute(0x42);
    EXPECT_EQ(_cpu.registers().D, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x43Test)
{
    // LD B,E
    _loadAndExecute(0x43);
    EXPECT_EQ(_cpu.registers().E, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x44Test)
{
    // LD B,H
    _loadAndExecute(0x44);
    EXPECT_EQ(_cpu.registers().H, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x45Test)
{
    // LD B,L
    _loadAndExecute(0x45);
    EXPECT_EQ(_cpu.registers().L, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x46Test)
{
    // LD B,(HL)
    _loadAndExecute(0x46);
    EXPECT_EQ(_mem[_cpu.registers().HL], _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x47Test)
{
    // LD B,A
    _loadAndExecute(0x47);
    EXPECT_EQ(_cpu.registers().A, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x48Test)
{
    // LD C,B
    _loadAndExecute(0x48);
    EXPECT_EQ(_cpu.registers().B, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x49Test)
{
    // LD C,C
    _loadAndExecute(0x49);
    EXPECT_EQ(_cpu.registers().C, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x4ATest)
{
    // LD C,D
    _loadAndExecute(0x4A);
    EXPECT_EQ(_cpu.registers().D, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x4BTest)
{
    // LD C,E
    _loadAndExecute(0x4B);
    EXPECT_EQ(_cpu.registers().E, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x4CTest)
{
    // LD C,H
    _loadAndExecute(0x4C);
    EXPECT_EQ(_cpu.registers().H, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x4DTest)
{
    // LD C,L
    _loadAndExecute(0x4D);
    EXPECT_EQ(_cpu.registers().L, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x4ETest)
{
    // LD C,(HL)
    _loadAndExecute(0x4E);
    EXPECT_EQ(_mem[_cpu.registers().HL], _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x4FTest)
{
    // LD C,A
    _loadAndExecute(0x4F);
    EXPECT_EQ(_cpu.registers().A, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x50Test)
{
    // LD D,B
    _loadAndExecute(0x50);
    EXPECT_EQ(_cpu.registers().B, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x51Test)
{
    // LD D,C
    _loadAndExecute(0x51);
    EXPECT_EQ(_cpu.registers().C, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x52Test)
{
    // LD D,D
    _loadAndExecute(0x52);
    EXPECT_EQ(_cpu.registers().D, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x53Test)
{
    // LD D,E
    _loadAndExecute(0x53);
    EXPECT_EQ(_cpu.registers().E, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x54Test)
{
    // LD D,H
    _loadAndExecute(0x54);
    EXPECT_EQ(_cpu.registers().H, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x55Test)
{
    // LD D,L
    _loadAndExecute(0x55);
    EXPECT_EQ(_cpu.registers().L, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x56Test)
{
    // LD D,(HL)
    _loadAndExecute(0x56);
    EXPECT_EQ(_mem[_cpu.registers().HL], _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x57Test)
{
    // LD D,A
    _loadAndExecute(0x57);
    EXPECT_EQ(_cpu.registers().A, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x58Test)
{
    // LD E,B
    _loadAndExecute(0x58);
    EXPECT_EQ(_cpu.registers().B, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x59Test)
{
    // LD E,C
    _loadAndExecute(0x59);
    EXPECT_EQ(_cpu.registers().C, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x5ATest)
{
    // LD E,D
    _loadAndExecute(0x5A);
    EXPECT_EQ(_cpu.registers().D, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x5BTest)
{
    // LD E,E
    _loadAndExecute(0x5B);
    EXPECT_EQ(_cpu.registers().E, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x5CTest)
{
    // LD E,H
    _loadAndExecute(0x5C);
    EXPECT_EQ(_cpu.registers().H, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x5DTest)
{
    // LD E,L
    _loadAndExecute(0x5D);
    EXPECT_EQ(_cpu.registers().L, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x5ETest)
{
    // LD E,(HL)
    _loadAndExecute(0x5E);
    EXPECT_EQ(_mem[_cpu.registers().HL], _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x5FTest)
{
    // LD E,A
    _loadAndExecute(0x5F);
    EXPECT_EQ(_cpu.registers().A, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x60Test)
{
    // LD H,B
    _loadAndExecute(0x60);
    EXPECT_EQ(_cpu.registers().B, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x61Test)
{
    // LD H,C
    _loadAndExecute(0x61);
    EXPECT_EQ(_cpu.registers().C, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x62Test)
{
    // LD H,D
    _loadAndExecute(0x62);
    EXPECT_EQ(_cpu.registers().D, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x63Test)
{
    // LD H,E
    _loadAndExecute(0x63);
    EXPECT_EQ(_cpu.registers().E, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x64Test)
{
    // LD H,H
    _loadAndExecute(0x64);
    EXPECT_EQ(_cpu.registers().H, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x65Test)
{
    // LD H,L
    _loadAndExecute(0x65);
    EXPECT_EQ(_cpu.registers().L, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x66Test)
{
    // LD H,(HL)
    _loadAndExecute(0x66);
    EXPECT_EQ(_mem[_cpu.registers().HL], _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x67Test)
{
    // LD H,A
    gb::Word val = _cpu.registers().A;
    _loadAndExecute(0x67);
    EXPECT_EQ(val, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x68Test)
{
    // LD L,B
    gb::Word val = _cpu.registers().B;
    _loadAndExecute(0x68);
    EXPECT_EQ(val, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x69Test)
{
    // LD L,C
    gb::Word val = _cpu.registers().C;
    _loadAndExecute(0x69);
    EXPECT_EQ(val, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x6ATest)
{
    // LD L,D
    gb::Word val = _cpu.registers().D;
    _loadAndExecute(0x6A);
    EXPECT_EQ(val, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x6BTest)
{
    // LD L,E
    gb::Word val = _cpu.registers().E;
    _loadAndExecute(0x6B);
    EXPECT_EQ(val, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x6CTest)
{
    // LD L,H
    gb::Word val = _cpu.registers().H;
    _loadAndExecute(0x6C);
    EXPECT_EQ(val, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x6DTest)
{
    // LD L,L
    gb::Word val = _cpu.registers().L;
    _loadAndExecute(0x6D);
    EXPECT_EQ(val, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x6ETest)
{
    // LD L,(HL)
    gb::Word val = _mem[_cpu.registers().HL];
    _loadAndExecute(0x6E);
    EXPECT_EQ(val, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x6FTest)
{
    // LD L,A
    gb::Word val = _cpu.registers().A;
    _loadAndExecute(0x6F);
    EXPECT_EQ(val, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x70Test)
{
    // LD (HL),B
    gb::Word val = _cpu.registers().B;
    _loadAndExecute(0x70);
    EXPECT_EQ(val, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x71Test)
{
    // LD (HL),C
    gb::Word val = _cpu.registers().C;
    _loadAndExecute(0x71);
    EXPECT_EQ(val, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x72Test)
{
    // LD (HL),D
    gb::Word val = _cpu.registers().D;
    _loadAndExecute(0x72);
    EXPECT_EQ(val, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x73Test)
{
    // LD (HL),E
    gb::Word val = _cpu.registers().E;
    _loadAndExecute(0x73);
    EXPECT_EQ(val, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x74Test)
{
    // LD (HL),H
    gb::Word val = _cpu.registers().H;
    _loadAndExecute(0x74);
    EXPECT_EQ(val, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x75Test)
{
    // LD (HL),L
    gb::Word val = _cpu.registers().L;
    _loadAndExecute(0x75);
    EXPECT_EQ(val, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x76Test)
{
    // HALT
    EXPECT_FALSE(_cpu.isHalted());
    _loadAndExecute(0x76);
    EXPECT_TRUE(_cpu.isHalted());
}

TEST_F(CpuTest, Opcode0x77Test)
{
    // LD (HL),A
    gb::Word val = _cpu.registers().A;
    _loadAndExecute(0x77);
    EXPECT_EQ(val, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x78Test)
{
    // LD A,B
    gb::Word val = _cpu.registers().B;
    _loadAndExecute(0x78);
    EXPECT_EQ(val, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x79Test)
{
    // LD A,C
    gb::Word val = _cpu.registers().C;
    _loadAndExecute(0x79);
    EXPECT_EQ(val, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x7ATest)
{
    // LD A,D
    gb::Word val = _cpu.registers().D;
    _loadAndExecute(0x7A);
    EXPECT_EQ(val, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x7BTest)
{
    // LD A,E
    gb::Word val = _cpu.registers().E;
    _loadAndExecute(0x7B);
    EXPECT_EQ(val, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x7CTest)
{
    // LD A,H
    gb::Word val = _cpu.registers().H;
    _loadAndExecute(0x7C);
    EXPECT_EQ(val, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x7DTest)
{
    // LD A,L
    gb::Word val = _cpu.registers().L;
    _loadAndExecute(0x7D);
    EXPECT_EQ(val, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x7ETest)
{
    // LD A,(HL)
    gb::Word val = _mem[_cpu.registers().HL];
    _loadAndExecute(0x7E);
    EXPECT_EQ(val, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x7FTest)
{
    // LD A,A
    gb::Word val = _cpu.registers().A;
    _loadAndExecute(0x7F);
    EXPECT_EQ(val, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0x80Test)
{
    // ADD A,B
    _cpu.registers().A = 0x07;
    _cpu.registers().B = 0xF0;
    _loadAndExecute(0x80);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x0F;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0x80);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xFF;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0x80);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x81Test)
{
    // ADD A,C
    _cpu.registers().A = 0x07;
    _cpu.registers().C = 0xF0;
    _loadAndExecute(0x81);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x0F;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0x81);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xFF;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0x81);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x82Test)
{
    // ADD A,D
    _cpu.registers().A = 0x07;
    _cpu.registers().D = 0xF0;
    _loadAndExecute(0x82);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x0F;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0x82);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xFF;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0x82);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x83Test)
{
    // ADD A,E
    _cpu.registers().A = 0x07;
    _cpu.registers().E = 0xF0;
    _loadAndExecute(0x83);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x0F;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0x83);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xFF;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0x83);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x84Test)
{
    // ADD A,H
    _cpu.registers().A = 0x07;
    _cpu.registers().H = 0xF0;
    _loadAndExecute(0x84);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x0F;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0x84);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xFF;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0x84);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x85Test)
{
    // ADD A,L
    _cpu.registers().A = 0x07;
    _cpu.registers().L = 0xF0;
    _loadAndExecute(0x85);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x0F;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0x85);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xFF;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0x85);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x86Test)
{
    // ADD A,(HL)
    _cpu.registers().A = 0x07;
    _mem[_cpu.registers().HL] = 0xF0;
    _loadAndExecute(0x86);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x0F;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x86);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xFF;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x86);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x87Test)
{
    // ADD A,A
    _cpu.registers().A = 0x02;
    _loadAndExecute(0x87);
    EXPECT_EQ(0x04, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x0F;
    _loadAndExecute(0x87);
    EXPECT_EQ(0x1E, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xFF;
    _loadAndExecute(0x87);
    EXPECT_EQ(0xFE, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,1);

    _cpu.registers().A = 0x00;
    _loadAndExecute(0x87);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0x88Test)
{
    // ADC A,B
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x07;
    _cpu.registers().B = 0xF0;
    _loadAndExecute(0x88);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x0F;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0x88);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0x88);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x06;
    _cpu.registers().B = 0xF0;
    _loadAndExecute(0x88);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x0E;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0x88);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFE;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0x88);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x89Test)
{
    // ADC A,C
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x07;
    _cpu.registers().C = 0xF0;
    _loadAndExecute(0x89);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x0F;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0x89);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0x89);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x06;
    _cpu.registers().C = 0xF0;
    _loadAndExecute(0x89);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x0E;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0x89);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFE;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0x89);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x8ATest)
{
    // ADC A,D
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x07;
    _cpu.registers().D = 0xF0;
    _loadAndExecute(0x8A);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x0F;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0x8A);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0x8A);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x06;
    _cpu.registers().D = 0xF0;
    _loadAndExecute(0x8A);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x0E;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0x8A);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFE;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0x8A);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x8BTest)
{
    // ADC A,E
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x07;
    _cpu.registers().E = 0xF0;
    _loadAndExecute(0x8B);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x0F;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0x8B);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0x8B);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x06;
    _cpu.registers().E = 0xF0;
    _loadAndExecute(0x8B);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x0E;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0x8B);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFE;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0x8B);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x8CTest)
{
    // ADC A,H
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x07;
    _cpu.registers().H = 0xF0;
    _loadAndExecute(0x8C);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x0F;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0x8C);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0x8C);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x06;
    _cpu.registers().H = 0xF0;
    _loadAndExecute(0x8C);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x0E;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0x8C);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFE;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0x8C);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x8DTest)
{
    // ADC A,L
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x07;
    _cpu.registers().L = 0xF0;
    _loadAndExecute(0x8D);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x0F;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0x8D);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0x8D);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x06;
    _cpu.registers().L = 0xF0;
    _loadAndExecute(0x8D);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x0E;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0x8D);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFE;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0x8D);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x8ETest)
{
    // ADC A,(HL)
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x07;
    _mem[_cpu.registers().HL] = 0xF0;
    _loadAndExecute(0x8E);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x0F;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x8E);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x8E);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x06;
    _mem[_cpu.registers().HL] = 0xF0;
    _loadAndExecute(0x8E);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x0E;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x8E);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFE;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x8E);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0x8FTest)
{
    // ADC A,A
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x07;
    _loadAndExecute(0x8F);
    EXPECT_EQ(0x0E, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x0F;
    _loadAndExecute(0x8F);
    EXPECT_EQ(0x1E, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0x8F);
    EXPECT_EQ(0xFE, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x06;
    _loadAndExecute(0x8F);
    EXPECT_EQ(0x0D, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x0E;
    _loadAndExecute(0x8F);
    EXPECT_EQ(0x1D, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0x90Test)
{
    // SUB B
    _cpu.registers().A = 0x0F;
    _cpu.registers().B = 0x02;
    _loadAndExecute(0x90);
    EXPECT_EQ(0x0D, _cpu.registers().A);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0x90);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0x90);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _cpu.registers().B = 0xF1;
    _loadAndExecute(0x90);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0x01;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0x90);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x91Test)
{
    // SUB C
    _cpu.registers().A = 0x0F;
    _cpu.registers().C = 0x02;
    _loadAndExecute(0x91);
    EXPECT_EQ(0x0D, _cpu.registers().A);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0x91);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0x91);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _cpu.registers().C = 0xF1;
    _loadAndExecute(0x91);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0x01;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0x91);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x92Test)
{
    // SUB D
    _cpu.registers().A = 0x0F;
    _cpu.registers().D = 0x02;
    _loadAndExecute(0x92);
    EXPECT_EQ(0x0D, _cpu.registers().A);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0x92);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0x92);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _cpu.registers().D = 0xF1;
    _loadAndExecute(0x92);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0x01;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0x92);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x93Test)
{
    // SUB E
    _cpu.registers().A = 0x0F;
    _cpu.registers().E = 0x02;
    _loadAndExecute(0x93);
    EXPECT_EQ(0x0D, _cpu.registers().A);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0x93);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0x93);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _cpu.registers().E = 0xF1;
    _loadAndExecute(0x93);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0x01;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0x93);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x94Test)
{
    // SUB H
    _cpu.registers().A = 0x0F;
    _cpu.registers().H = 0x02;
    _loadAndExecute(0x94);
    EXPECT_EQ(0x0D, _cpu.registers().A);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0x94);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0x94);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _cpu.registers().H = 0xF1;
    _loadAndExecute(0x94);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0x01;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0x94);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x95Test)
{
    // SUB L
    _cpu.registers().A = 0x0F;
    _cpu.registers().L = 0x02;
    _loadAndExecute(0x95);
    EXPECT_EQ(0x0D, _cpu.registers().A);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0x95);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0x95);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _cpu.registers().L = 0xF1;
    _loadAndExecute(0x95);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0x01;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0x95);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x96Test)
{
    // SUB (HL)
    _cpu.registers().A = 0x0F;
    _mem[_cpu.registers().HL] = 0x02;
    _loadAndExecute(0x96);
    EXPECT_EQ(0x0D, _cpu.registers().A);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x96);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x96);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _mem[_cpu.registers().HL] = 0xF1;
    _loadAndExecute(0x96);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0x01;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x96);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x97Test)
{
    // SUB A
    _cpu.registers().A = 0xF1;
    _loadAndExecute(0x97);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0x02;
    _loadAndExecute(0x97);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x98Test)
{
    // SBC A,B
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xF0;
    _cpu.registers().B = 0x07;
    _loadAndExecute(0x98);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x10;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0x98);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0x98);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _cpu.registers().B = 0xFF;
    _loadAndExecute(0x98);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xF0;
    _cpu.registers().B = 0x06;
    _loadAndExecute(0x98);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x11;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0x98);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x02;
    _cpu.registers().B = 0x02;
    _loadAndExecute(0x98);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFF;
    _cpu.registers().B = 0xFE;
    _loadAndExecute(0x98);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x99Test)
{
    // SBC A,C
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xF0;
    _cpu.registers().C = 0x07;
    _loadAndExecute(0x99);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x10;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0x99);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0x99);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _cpu.registers().C = 0xFF;
    _loadAndExecute(0x99);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xF0;
    _cpu.registers().C = 0x06;
    _loadAndExecute(0x99);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x11;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0x99);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x02;
    _cpu.registers().C = 0x02;
    _loadAndExecute(0x99);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFF;
    _cpu.registers().C = 0xFE;
    _loadAndExecute(0x99);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x9ATest)
{
    // SBC A,D
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xF0;
    _cpu.registers().D = 0x07;
    _loadAndExecute(0x9A);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x10;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0x9A);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0x9A);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _cpu.registers().D = 0xFF;
    _loadAndExecute(0x9A);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xF0;
    _cpu.registers().D = 0x06;
    _loadAndExecute(0x9A);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x11;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0x9A);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x02;
    _cpu.registers().D = 0x02;
    _loadAndExecute(0x9A);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFF;
    _cpu.registers().D = 0xFE;
    _loadAndExecute(0x9A);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x9BTest)
{
    // SBC A,E
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xF0;
    _cpu.registers().E = 0x07;
    _loadAndExecute(0x9B);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x10;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0x9B);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0x9B);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _cpu.registers().E = 0xFF;
    _loadAndExecute(0x9B);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xF0;
    _cpu.registers().E = 0x06;
    _loadAndExecute(0x9B);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x11;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0x9B);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x02;
    _cpu.registers().E = 0x02;
    _loadAndExecute(0x9B);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFF;
    _cpu.registers().E = 0xFE;
    _loadAndExecute(0x9B);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x9CTest)
{
    // SBC A,H
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xF0;
    _cpu.registers().H = 0x07;
    _loadAndExecute(0x9C);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x10;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0x9C);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0x9C);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _cpu.registers().H = 0xFF;
    _loadAndExecute(0x9C);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xF0;
    _cpu.registers().H = 0x06;
    _loadAndExecute(0x9C);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x11;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0x9C);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x02;
    _cpu.registers().H = 0x02;
    _loadAndExecute(0x9C);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFF;
    _cpu.registers().H = 0xFE;
    _loadAndExecute(0x9C);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x9DTest)
{
    // SBC A,L
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xF0;
    _cpu.registers().L = 0x07;
    _loadAndExecute(0x9D);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x10;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0x9D);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0x9D);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _cpu.registers().L = 0xFF;
    _loadAndExecute(0x9D);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xF0;
    _cpu.registers().L = 0x06;
    _loadAndExecute(0x9D);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x11;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0x9D);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x02;
    _cpu.registers().L = 0x02;
    _loadAndExecute(0x9D);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFF;
    _cpu.registers().L = 0xFE;
    _loadAndExecute(0x9D);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x9ETest)
{
    // SBC A,(HL)
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xF0;
    _mem[_cpu.registers().HL] = 0x07;
    _loadAndExecute(0x9E);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x10;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x9E);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x9E);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _mem[_cpu.registers().HL] = 0xFF;
    _loadAndExecute(0x9E);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xF0;
    _mem[_cpu.registers().HL] = 0x06;
    _loadAndExecute(0x9E);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x11;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0x9E);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x02;
    _mem[_cpu.registers().HL] = 0x02;
    _loadAndExecute(0x9E);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFF;
    _mem[_cpu.registers().HL] = 0xFE;
    _loadAndExecute(0x9E);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0x9FTest)
{
    // SBC A,A
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xF0;
    _loadAndExecute(0x9F);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x10;
    _loadAndExecute(0x9F);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _loadAndExecute(0x9F);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0x9F);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xF0;
    _loadAndExecute(0x9F);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x11;
    _loadAndExecute(0x9F);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x02;
    _loadAndExecute(0x9F);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0x9F);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,0,0);
}

TEST_F(CpuTest, Opcode0xA0Test)
{
    // AND B
    _cpu.registers().A = 0x0C;
    _cpu.registers().B = 0x18;
    _loadAndExecute(0xA0);
    EXPECT_EQ(0x08, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xF0;
    _cpu.registers().B = 0x0F;
    _loadAndExecute(0xA0);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0xA1Test)
{
    // AND C
    _cpu.registers().A = 0x0C;
    _cpu.registers().C = 0x18;
    _loadAndExecute(0xA1);
    EXPECT_EQ(0x08, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xF0;
    _cpu.registers().C = 0x0F;
    _loadAndExecute(0xA1);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0xA2Test)
{
    // AND D
    _cpu.registers().A = 0x0C;
    _cpu.registers().D = 0x18;
    _loadAndExecute(0xA2);
    EXPECT_EQ(0x08, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xF0;
    _cpu.registers().D = 0x0F;
    _loadAndExecute(0xA2);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0xA3Test)
{
    // AND E
    _cpu.registers().A = 0x0C;
    _cpu.registers().E = 0x18;
    _loadAndExecute(0xA3);
    EXPECT_EQ(0x08, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xF0;
    _cpu.registers().E = 0x0F;
    _loadAndExecute(0xA3);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0xA4Test)
{
    // AND H
    _cpu.registers().A = 0x0C;
    _cpu.registers().H = 0x18;
    _loadAndExecute(0xA4);
    EXPECT_EQ(0x08, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xF0;
    _cpu.registers().H = 0x0F;
    _loadAndExecute(0xA4);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0xA5Test)
{
    // AND L
    _cpu.registers().A = 0x0C;
    _cpu.registers().L = 0x18;
    _loadAndExecute(0xA5);
    EXPECT_EQ(0x08, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xF0;
    _cpu.registers().L = 0x0F;
    _loadAndExecute(0xA5);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0xA6Test)
{
    // AND (HL)
    _cpu.registers().A = 0x0C;
    _mem[_cpu.registers().HL] = 0x18;
    _loadAndExecute(0xA6);
    EXPECT_EQ(0x08, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xF0;
    _mem[_cpu.registers().HL] = 0x0F;
    _loadAndExecute(0xA6);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0xA7Test)
{
    // AND A
    _cpu.registers().A = 0x0C;
    _loadAndExecute(0xA7);
    EXPECT_EQ(0x0C, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0x00;
    _loadAndExecute(0xA7);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0xA8Test)
{
    // XOR B
    _cpu.registers().A = 0x0C;
    _cpu.registers().B = 0x18;
    _loadAndExecute(0xA8);
    EXPECT_EQ(0x14, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0xF0;
    _cpu.registers().B = 0xF0;
    _loadAndExecute(0xA8);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xA9Test)
{
    // XOR C
    _cpu.registers().A = 0x0C;
    _cpu.registers().C = 0x18;
    _loadAndExecute(0xA9);
    EXPECT_EQ(0x14, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0xF0;
    _cpu.registers().C = 0xF0;
    _loadAndExecute(0xA9);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xAATest)
{
    // XOR D
    _cpu.registers().A = 0x0C;
    _cpu.registers().D = 0x18;
    _loadAndExecute(0xAA);
    EXPECT_EQ(0x14, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0xF0;
    _cpu.registers().D = 0xF0;
    _loadAndExecute(0xAA);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xABTest)
{
    // XOR E
    _cpu.registers().A = 0x0C;
    _cpu.registers().E = 0x18;
    _loadAndExecute(0xAB);
    EXPECT_EQ(0x14, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0xF0;
    _cpu.registers().E = 0xF0;
    _loadAndExecute(0xAB);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xACTest)
{
    // XOR H
    _cpu.registers().A = 0x0C;
    _cpu.registers().H = 0x18;
    _loadAndExecute(0xAC);
    EXPECT_EQ(0x14, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0xF0;
    _cpu.registers().H = 0xF0;
    _loadAndExecute(0xAC);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xADTest)
{
    // XOR L
    _cpu.registers().A = 0x0C;
    _cpu.registers().L = 0x18;
    _loadAndExecute(0xAD);
    EXPECT_EQ(0x14, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0xF0;
    _cpu.registers().L = 0xF0;
    _loadAndExecute(0xAD);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xAETest)
{
    // XOR (HL)
    _cpu.registers().A = 0x0C;
    _mem[_cpu.registers().HL] = 0x18;
    _loadAndExecute(0xAE);
    EXPECT_EQ(0x14, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0xF0;
    _mem[_cpu.registers().HL] = 0xF0;
    _loadAndExecute(0xAE);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xAFTest)
{
    // XOR A
    _cpu.registers().A = 0x0C;
    _loadAndExecute(0xAF);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xB0Test)
{
    // OR B
    _cpu.registers().A = 0x0C;
    _cpu.registers().B = 0x18;
    _loadAndExecute(0xB0);
    EXPECT_EQ(0x1C, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xB0);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xB1Test)
{
    // OR C
    _cpu.registers().A = 0x0C;
    _cpu.registers().C = 0x18;
    _loadAndExecute(0xB1);
    EXPECT_EQ(0x1C, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xB1);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xB2Test)
{
    // OR D
    _cpu.registers().A = 0x0C;
    _cpu.registers().D = 0x18;
    _loadAndExecute(0xB2);
    EXPECT_EQ(0x1C, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xB2);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xB3Test)
{
    // OR E
    _cpu.registers().A = 0x0C;
    _cpu.registers().E = 0x18;
    _loadAndExecute(0xB3);
    EXPECT_EQ(0x1C, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xB3);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xB4Test)
{
    // OR H
    _cpu.registers().A = 0x0C;
    _cpu.registers().H = 0x18;
    _loadAndExecute(0xB4);
    EXPECT_EQ(0x1C, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xB4);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xB5Test)
{
    // OR L
    _cpu.registers().A = 0x0C;
    _cpu.registers().L = 0x18;
    _loadAndExecute(0xB5);
    EXPECT_EQ(0x1C, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xB5);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xB6Test)
{
    // OR (HL)
    _cpu.registers().A = 0x0C;
    _mem[_cpu.registers().HL] = 0x18;
    _loadAndExecute(0xB6);
    EXPECT_EQ(0x1C, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x00;
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xB6);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xB7Test)
{
    // OR A
    _cpu.registers().A = 0x0C;
    _loadAndExecute(0xB7);
    EXPECT_EQ(0x0C, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x00;
    _loadAndExecute(0xB7);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xB8Test)
{
    // CP B
    _cpu.registers().A = 0x0F;
    _cpu.registers().B = 0x02;
    _loadAndExecute(0xB8);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0xB8);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().B = 0x01;
    _loadAndExecute(0xB8);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _cpu.registers().B = 0xF1;
    _loadAndExecute(0xB8);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0xB9Test)
{
    // CP C
    _cpu.registers().A = 0x0F;
    _cpu.registers().C = 0x02;
    _loadAndExecute(0xB9);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0xB9);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().C = 0x01;
    _loadAndExecute(0xB9);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _cpu.registers().C = 0xF1;
    _loadAndExecute(0xB9);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0xBATest)
{
    // CP D
    _cpu.registers().A = 0x0F;
    _cpu.registers().D = 0x02;
    _loadAndExecute(0xBA);
    EXPECT_FLAGS(0,1,0,0);
    
    _cpu.registers().A = 0x10;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0xBA);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().D = 0x01;
    _loadAndExecute(0xBA);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _cpu.registers().D = 0xF1;
    _loadAndExecute(0xBA);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0xBBTest)
{
    // CP E
    _cpu.registers().A = 0x0F;
    _cpu.registers().E = 0x02;
    _loadAndExecute(0xBB);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0xBB);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().E = 0x01;
    _loadAndExecute(0xBB);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _cpu.registers().E = 0xF1;
    _loadAndExecute(0xBB);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0xBCTest)
{
    // CP H
    _cpu.registers().A = 0x0F;
    _cpu.registers().H = 0x02;
    _loadAndExecute(0xBC);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0xBC);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().H = 0x01;
    _loadAndExecute(0xBC);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _cpu.registers().H = 0xF1;
    _loadAndExecute(0xBC);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0xBDTest)
{
    // CP L
    _cpu.registers().A = 0x0F;
    _cpu.registers().L = 0x02;
    _loadAndExecute(0xBD);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0xBD);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _cpu.registers().L = 0x01;
    _loadAndExecute(0xBD);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _cpu.registers().L = 0xF1;
    _loadAndExecute(0xBD);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0xBETest)
{
    // CP (HL)
    _cpu.registers().A = 0x0F;
    _mem[_cpu.registers().HL] = 0x02;
    _loadAndExecute(0xBE);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0xBE);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0xBE);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _mem[_cpu.registers().HL] = 0xF1;
    _loadAndExecute(0xBE);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0xBFTest)
{
    // CP A
    _cpu.registers().A = 0x0F;
    _loadAndExecute(0xBF);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0xF1;
    _loadAndExecute(0xBF);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0xC0Test)
{
    // RET NZ
    _cpu.registers().F = 0x00;
    _cpu.registers().SP = 0xFF00;
    _mem[0xFF00] = 0xAA;
    _mem[0xFF01] = 0xFF;
    _loadAndExecute(0xC0);
    EXPECT_EQ(0xFFAA, _cpu.registers().PC);
    EXPECT_EQ(0xFF02, _cpu.registers().SP);

    _cpu.registers().PC = 0x0000;

    _cpu.registers().F = 0xFF;
    _cpu.registers().SP = 0xFF00;
    _mem[0xFF00] = 0xAA;
    _mem[0xFF01] = 0xFF;
    _loadAndExecute(0xC0);
    EXPECT_EQ(0x0001, _cpu.registers().PC);
    EXPECT_EQ(0xFF00, _cpu.registers().SP);
}

TEST_F(CpuTest, Opcode0xC1Test)
{
    // POP BC
    _cpu.registers().SP = 0x0038;
    _mem[0x0039] = 0xAB;
    _mem[0x0038] = 0xFE;
    _loadAndExecute(0xC1);
    EXPECT_EQ(0xAB, _cpu.registers().B);
    EXPECT_EQ(0xFE, _cpu.registers().C);
    EXPECT_EQ(_cpu.registers().SP, 0x003A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xC2Test)
{
    // JP NZ,(nn)
    _cpu.registers().F = 0x00;
    _loadAndExecute(0xC2, 0xFF, 0xAA);
    EXPECT_EQ(0xAAFF, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().PC = 0x0000;

    _cpu.registers().F = 0xFF;
    _loadAndExecute(0xC2, 0xFF, 0xAA);
    EXPECT_EQ(0x0003, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);
}

TEST_F(CpuTest, Opcode0xC3Test)
{
    // JP (nn)
    _loadAndExecute(0xC3, 0xFF, 0xAA);
    EXPECT_EQ(0xAAFF, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xC4Test)
{
    // CALL NZ,(nn)
    _cpu.registers().F = 0x00;
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xC4, 0xAA, 0x22);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x22AA, _cpu.registers().PC);
    EXPECT_EQ(0x11, _mem[0xFEFF]);
    EXPECT_EQ(0xAD, _mem[0xFEFE]);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xC4, 0xAA, 0x22);
    EXPECT_EQ(0xFF00, _cpu.registers().SP);
    EXPECT_EQ(0x11AD, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);
}

TEST_F(CpuTest, Opcode0xC5Test)
{
    // PUSH BC
    _cpu.registers().SP = 0x003A;
    _cpu.registers().BC = 0xABFE;
    _loadAndExecute(0xC5);
    EXPECT_EQ(_cpu.registers().B, _mem[0x0039]);
    EXPECT_EQ(_cpu.registers().C, _mem[0x0038]);
    EXPECT_EQ(_cpu.registers().SP, 0x0038);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xC6Test)
{
    // ADD A,n
    _cpu.registers().A = 0x07;
    _loadAndExecute(0xC6, 0xF0);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x0F;
    _loadAndExecute(0xC6, 0x01);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xC6, 0x02);
    EXPECT_EQ(0x01, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,1);

    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xC6, 0x01);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0xC7Test)
{
    // RST 0H
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xC7);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x0000, _cpu.registers().PC);
    EXPECT_EQ(0x11, (int)_mem[0xFEFF]);
    EXPECT_EQ(0xAB, (int)_mem[0xFEFE]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xC8Test)
{
    // RET Z
    _cpu.registers().F = 0xFF;
    _cpu.registers().SP = 0xFF00;
    _mem[0xFF00] = 0xAA;
    _mem[0xFF01] = 0xFF;
    _loadAndExecute(0xC8);
    EXPECT_EQ(0xFFAA, _cpu.registers().PC);
    EXPECT_EQ(0xFF02, _cpu.registers().SP);

    _cpu.registers().PC = 0x0000;

    _cpu.registers().F = 0x00;
    _cpu.registers().SP = 0xFF00;
    _mem[0xFF00] = 0xAA;
    _mem[0xFF01] = 0xFF;
    _loadAndExecute(0xC8);
    EXPECT_EQ(0x0001, _cpu.registers().PC);
    EXPECT_EQ(0xFF00, _cpu.registers().SP);
}

TEST_F(CpuTest, Opcode0xC9Test)
{
    // RET
    _cpu.registers().SP = 0xFF00;
    _mem[0xFF00] = 0xAA;
    _mem[0xFF01] = 0xFF;
    _loadAndExecute(0xC9);
    EXPECT_EQ(0xFFAA, _cpu.registers().PC);
    EXPECT_EQ(0xFF02, _cpu.registers().SP);

}

TEST_F(CpuTest, Opcode0xCATest)
{
    // JP Z,(nn)
    _cpu.registers().F = 0xFF;
    _loadAndExecute(0xCA, 0xFF, 0xAA);
    EXPECT_EQ(0xAAFF, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);

    _cpu.registers().PC = 0x0000;

    _cpu.registers().F = 0x00;
    _loadAndExecute(0xCA, 0xFF, 0xAA);
    EXPECT_EQ(0x0003, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB00Test)
{
    // RLC B
    _cpu.registers().F = 0x00;
    _cpu.registers().B = 137; // 10001001
    _loadAndExecute(0xCB, 0x00);
    EXPECT_EQ(19, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().B = 9; // 00001001
    _loadAndExecute(0xCB, 0x00);
    EXPECT_EQ(18, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0x00);
    EXPECT_EQ(0x00, _cpu.registers().B);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB01Test)
{
    // RLC C
    _cpu.registers().F = 0x00;
    _cpu.registers().C = 137; // 10001001
    _loadAndExecute(0xCB, 0x01);
    EXPECT_EQ(19, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().C = 9; // 00001001
    _loadAndExecute(0xCB, 0x01);
    EXPECT_EQ(18, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0x01);
    EXPECT_EQ(0x00, _cpu.registers().C);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB02Test)
{
    // RLC D
    _cpu.registers().F = 0x00;
    _cpu.registers().D = 137; // 10001001
    _loadAndExecute(0xCB, 0x02);
    EXPECT_EQ(19, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().D = 9; // 00001001
    _loadAndExecute(0xCB, 0x02);
    EXPECT_EQ(18, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0x02);
    EXPECT_EQ(0x00, _cpu.registers().D);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB03Test)
{
    // RLC E
    _cpu.registers().F = 0x00;
    _cpu.registers().E = 137; // 10001001
    _loadAndExecute(0xCB, 0x03);
    EXPECT_EQ(19, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().E = 9; // 00001001
    _loadAndExecute(0xCB, 0x03);
    EXPECT_EQ(18, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0x03);
    EXPECT_EQ(0x00, _cpu.registers().E);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB04Test)
{
    // RLC H
    _cpu.registers().F = 0x00;
    _cpu.registers().H = 137; // 10001001
    _loadAndExecute(0xCB, 0x04);
    EXPECT_EQ(19, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().H = 9; // 00001001
    _loadAndExecute(0xCB, 0x04);
    EXPECT_EQ(18, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0x04);
    EXPECT_EQ(0x00, _cpu.registers().H);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB05Test)
{
    // RLC L
    _cpu.registers().F = 0x00;
    _cpu.registers().L = 137; // 10001001
    _loadAndExecute(0xCB, 0x05);
    EXPECT_EQ(19, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().L = 9; // 00001001
    _loadAndExecute(0xCB, 0x05);
    EXPECT_EQ(18, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0x05);
    EXPECT_EQ(0x00, _cpu.registers().L);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB06Test)
{
    // RLC (HL)
    _cpu.registers().F = 0x00;
    _mem[_cpu.registers().HL] = 137; // 10001001
    _loadAndExecute(0xCB, 0x06);
    EXPECT_EQ(19, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _mem[_cpu.registers().HL] = 9; // 00001001
    _loadAndExecute(0xCB, 0x06);
    EXPECT_EQ(18, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0x06);
    EXPECT_EQ(0x00, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB07Test)
{
    // RLC A
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0xCB, 0x07);
    EXPECT_EQ(19, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 9; // 00001001
    _loadAndExecute(0xCB, 0x07);
    EXPECT_EQ(18, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0x07);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB08Test)
{
    // RRC B
    _cpu.registers().F = 0x00;
    _cpu.registers().B = 137; // 10001001
    _loadAndExecute(0xCB, 0x08);
    EXPECT_EQ(196, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().B = 136; // 10001000
    _loadAndExecute(0xCB, 0x08);
    EXPECT_EQ(68, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0x08);
    EXPECT_EQ(0x00, _cpu.registers().B);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB09Test)
{
    // RRC C
    _cpu.registers().F = 0x00;
    _cpu.registers().C = 137; // 10001001
    _loadAndExecute(0xCB, 0x09);
    EXPECT_EQ(196, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().C = 136; // 10001000
    _loadAndExecute(0xCB, 0x09);
    EXPECT_EQ(68, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0x09);
    EXPECT_EQ(0x00, _cpu.registers().C);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB0ATest)
{
    // RRC D
    _cpu.registers().F = 0x00;
    _cpu.registers().D = 137; // 10001001
    _loadAndExecute(0xCB, 0x0A);
    EXPECT_EQ(196, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().D = 136; // 10001000
    _loadAndExecute(0xCB, 0x0A);
    EXPECT_EQ(68, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0x0A);
    EXPECT_EQ(0x00, _cpu.registers().D);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB0BTest)
{
    // RRC E
    _cpu.registers().F = 0x00;
    _cpu.registers().E = 137; // 10001001
    _loadAndExecute(0xCB, 0x0B);
    EXPECT_EQ(196, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().E = 136; // 10001000
    _loadAndExecute(0xCB, 0x0B);
    EXPECT_EQ(68, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0x0B);
    EXPECT_EQ(0x00, _cpu.registers().E);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB0CTest)
{
    // RRC H
    _cpu.registers().F = 0x00;
    _cpu.registers().H = 137; // 10001001
    _loadAndExecute(0xCB, 0x0C);
    EXPECT_EQ(196, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().H = 136; // 10001000
    _loadAndExecute(0xCB, 0x0C);
    EXPECT_EQ(68, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0x0C);
    EXPECT_EQ(0x00, _cpu.registers().H);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB0DTest)
{
    // RRC L
    _cpu.registers().F = 0x00;
    _cpu.registers().L = 137; // 10001001
    _loadAndExecute(0xCB, 0x0D);
    EXPECT_EQ(196, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().L = 136; // 10001000
    _loadAndExecute(0xCB, 0x0D);
    EXPECT_EQ(68, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0x0D);
    EXPECT_EQ(0x00, _cpu.registers().L);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB0ETest)
{
    // RRC (HL)
    _cpu.registers().F = 0x00;
    _mem[_cpu.registers().HL] = 137; // 10001001
    _loadAndExecute(0xCB, 0x0E);
    EXPECT_EQ(196, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _mem[_cpu.registers().HL] = 136; // 10001000
    _loadAndExecute(0xCB, 0x0E);
    EXPECT_EQ(68, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0x0E);
    EXPECT_EQ(0x00, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB0FTest)
{
    // RRC A
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0xCB, 0x0F);
    EXPECT_EQ(196, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 136; // 10001000
    _loadAndExecute(0xCB, 0x0F);
    EXPECT_EQ(68, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0x0F);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB10Test)
{
    // RL B
    _cpu.registers().F = 0xFF;
    _cpu.registers().B = 137; // 10001001
    _loadAndExecute(0xCB, 0x10);
    EXPECT_EQ(19, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().B = 137; // 10001001
    _loadAndExecute(0xCB, 0x10);
    EXPECT_EQ(18, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().B = 8; // 00001000
    _loadAndExecute(0xCB, 0x10);
    EXPECT_EQ(17, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().B = 8; // 00001000
    _loadAndExecute(0xCB, 0x10);
    EXPECT_EQ(16, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0x10);
    EXPECT_EQ(0x00, _cpu.registers().B);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB11Test)
{
    // RL C
    _cpu.registers().F = 0xFF;
    _cpu.registers().C = 137; // 10001001
    _loadAndExecute(0xCB, 0x11);
    EXPECT_EQ(19, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().C = 137; // 10001001
    _loadAndExecute(0xCB, 0x11);
    EXPECT_EQ(18, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().C = 8; // 00001000
    _loadAndExecute(0xCB, 0x11);
    EXPECT_EQ(17, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().C = 8; // 00001000
    _loadAndExecute(0xCB, 0x11);
    EXPECT_EQ(16, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0x11);
    EXPECT_EQ(0x00, _cpu.registers().C);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB12Test)
{
    // RL D
    _cpu.registers().F = 0xFF;
    _cpu.registers().D = 137; // 10001001
    _loadAndExecute(0xCB, 0x12);
    EXPECT_EQ(19, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().D = 137; // 10001001
    _loadAndExecute(0xCB, 0x12);
    EXPECT_EQ(18, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().D = 8; // 00001000
    _loadAndExecute(0xCB, 0x12);
    EXPECT_EQ(17, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().D = 8; // 00001000
    _loadAndExecute(0xCB, 0x12);
    EXPECT_EQ(16, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0x12);
    EXPECT_EQ(0x00, _cpu.registers().D);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB13Test)
{
    // RL E
    _cpu.registers().F = 0xFF;
    _cpu.registers().E = 137; // 10001001
    _loadAndExecute(0xCB, 0x13);
    EXPECT_EQ(19, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().E = 137; // 10001001
    _loadAndExecute(0xCB, 0x13);
    EXPECT_EQ(18, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().E = 8; // 00001000
    _loadAndExecute(0xCB, 0x13);
    EXPECT_EQ(17, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().E = 8; // 00001000
    _loadAndExecute(0xCB, 0x13);
    EXPECT_EQ(16, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0x13);
    EXPECT_EQ(0x00, _cpu.registers().E);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB14Test)
{
    // RL H
    _cpu.registers().F = 0xFF;
    _cpu.registers().H = 137; // 10001001
    _loadAndExecute(0xCB, 0x14);
    EXPECT_EQ(19, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().H = 137; // 10001001
    _loadAndExecute(0xCB, 0x14);
    EXPECT_EQ(18, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().H = 8; // 00001000
    _loadAndExecute(0xCB, 0x14);
    EXPECT_EQ(17, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().H = 8; // 00001000
    _loadAndExecute(0xCB, 0x14);
    EXPECT_EQ(16, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0x14);
    EXPECT_EQ(0x00, _cpu.registers().H);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB15Test)
{
    // RL L
    _cpu.registers().F = 0xFF;
    _cpu.registers().L = 137; // 10001001
    _loadAndExecute(0xCB, 0x15);
    EXPECT_EQ(19, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().L = 137; // 10001001
    _loadAndExecute(0xCB, 0x15);
    EXPECT_EQ(18, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().L = 8; // 00001000
    _loadAndExecute(0xCB, 0x15);
    EXPECT_EQ(17, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().L = 8; // 00001000
    _loadAndExecute(0xCB, 0x15);
    EXPECT_EQ(16, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0x15);
    EXPECT_EQ(0x00, _cpu.registers().L);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB16Test)
{
    // RL (HL)
    _cpu.registers().F = 0xFF;
    _mem[_cpu.registers().HL] = 137; // 10001001
    _loadAndExecute(0xCB, 0x16);
    EXPECT_EQ(19, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _mem[_cpu.registers().HL] = 137; // 10001001
    _loadAndExecute(0xCB, 0x16);
    EXPECT_EQ(18, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _mem[_cpu.registers().HL] = 8; // 00001000
    _loadAndExecute(0xCB, 0x16);
    EXPECT_EQ(17, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _mem[_cpu.registers().HL] = 8; // 00001000
    _loadAndExecute(0xCB, 0x16);
    EXPECT_EQ(16, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0x16);
    EXPECT_EQ(0x00, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB17Test)
{
    // RL A
    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0xCB, 0x17);
    EXPECT_EQ(19, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0xCB, 0x17);
    EXPECT_EQ(18, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 8; // 00001000
    _loadAndExecute(0xCB, 0x17);
    EXPECT_EQ(17, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 8; // 00001000
    _loadAndExecute(0xCB, 0x17);
    EXPECT_EQ(16, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0x17);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB18Test)
{
    // RR B
    _cpu.registers().F = 0xFF;
    _cpu.registers().B = 137; // 10001001
    _loadAndExecute(0xCB, 0x18);
    EXPECT_EQ(196, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().B = 137; // 10001001
    _loadAndExecute(0xCB, 0x18);
    EXPECT_EQ(68, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().B = 136; // 10001000
    _loadAndExecute(0xCB, 0x18);
    EXPECT_EQ(196, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().B = 136; // 10001000
    _loadAndExecute(0xCB, 0x18);
    EXPECT_EQ(68, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0x18);
    EXPECT_EQ(0x00, _cpu.registers().B);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB19Test)
{
    // RR C
    _cpu.registers().F = 0xFF;
    _cpu.registers().C = 137; // 10001001
    _loadAndExecute(0xCB, 0x19);
    EXPECT_EQ(196, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().C = 137; // 10001001
    _loadAndExecute(0xCB, 0x19);
    EXPECT_EQ(68, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().C = 136; // 10001000
    _loadAndExecute(0xCB, 0x19);
    EXPECT_EQ(196, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().C = 136; // 10001000
    _loadAndExecute(0xCB, 0x19);
    EXPECT_EQ(68, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0x19);
    EXPECT_EQ(0x00, _cpu.registers().C);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB1ATest)
{
    // RR D
    _cpu.registers().F = 0xFF;
    _cpu.registers().D = 137; // 10001001
    _loadAndExecute(0xCB, 0x1A);
    EXPECT_EQ(196, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().D = 137; // 10001001
    _loadAndExecute(0xCB, 0x1A);
    EXPECT_EQ(68, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().D = 136; // 10001000
    _loadAndExecute(0xCB, 0x1A);
    EXPECT_EQ(196, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().D = 136; // 10001000
    _loadAndExecute(0xCB, 0x1A);
    EXPECT_EQ(68, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0x1A);
    EXPECT_EQ(0x00, _cpu.registers().D);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB1BTest)
{
    // RR E
    _cpu.registers().F = 0xFF;
    _cpu.registers().E = 137; // 10001001
    _loadAndExecute(0xCB, 0x1B);
    EXPECT_EQ(196, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().E = 137; // 10001001
    _loadAndExecute(0xCB, 0x1B);
    EXPECT_EQ(68, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().E = 136; // 10001000
    _loadAndExecute(0xCB, 0x1B);
    EXPECT_EQ(196, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().E = 136; // 10001000
    _loadAndExecute(0xCB, 0x1B);
    EXPECT_EQ(68, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0x1B);
    EXPECT_EQ(0x00, _cpu.registers().E);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB1CTest)
{
    // RR H
    _cpu.registers().F = 0xFF;
    _cpu.registers().H = 137; // 10001001
    _loadAndExecute(0xCB, 0x1C);
    EXPECT_EQ(196, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().H = 137; // 10001001
    _loadAndExecute(0xCB, 0x1C);
    EXPECT_EQ(68, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().H = 136; // 10001000
    _loadAndExecute(0xCB, 0x1C);
    EXPECT_EQ(196, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().H = 136; // 10001000
    _loadAndExecute(0xCB, 0x1C);
    EXPECT_EQ(68, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0x1C);
    EXPECT_EQ(0x00, _cpu.registers().H);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB1DTest)
{
    // RR L
    _cpu.registers().F = 0xFF;
    _cpu.registers().L = 137; // 10001001
    _loadAndExecute(0xCB, 0x1D);
    EXPECT_EQ(196, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().L = 137; // 10001001
    _loadAndExecute(0xCB, 0x1D);
    EXPECT_EQ(68, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().L = 136; // 10001000
    _loadAndExecute(0xCB, 0x1D);
    EXPECT_EQ(196, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().L = 136; // 10001000
    _loadAndExecute(0xCB, 0x1D);
    EXPECT_EQ(68, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0x1D);
    EXPECT_EQ(0x00, _cpu.registers().L);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB1ETest)
{
    // RR (HL)
    _cpu.registers().F = 0xFF;
    _mem[_cpu.registers().HL] = 137; // 10001001
    _loadAndExecute(0xCB, 0x1E);
    EXPECT_EQ(196, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _mem[_cpu.registers().HL] = 137; // 10001001
    _loadAndExecute(0xCB, 0x1E);
    EXPECT_EQ(68, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _mem[_cpu.registers().HL] = 136; // 10001000
    _loadAndExecute(0xCB, 0x1E);
    EXPECT_EQ(196, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _mem[_cpu.registers().HL] = 136; // 10001000
    _loadAndExecute(0xCB, 0x1E);
    EXPECT_EQ(68, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0x1E);
    EXPECT_EQ(0x00, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB1FTest)
{
    // RR A
    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0xCB, 0x1F);
    EXPECT_EQ(196, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0xCB, 0x1F);
    EXPECT_EQ(68, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 136; // 10001000
    _loadAndExecute(0xCB, 0x1F);
    EXPECT_EQ(196, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 136; // 10001000
    _loadAndExecute(0xCB, 0x1F);
    EXPECT_EQ(68, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0x1F);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB20Test)
{
    // SLA B
    _cpu.registers().F = 0x00;
    _cpu.registers().B = 137; // 10001001
    _loadAndExecute(0xCB, 0x20);
    EXPECT_EQ(18, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().B = 8; // 00001000
    _loadAndExecute(0xCB, 0x20);
    EXPECT_EQ(16, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0x20);
    EXPECT_EQ(0x00, _cpu.registers().B);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB21Test)
{
    // SLA C
    _cpu.registers().C = 137; // 10001001
    _loadAndExecute(0xCB, 0x21);
    EXPECT_EQ(18, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().C = 8; // 00001000
    _loadAndExecute(0xCB, 0x21);
    EXPECT_EQ(16, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0x21);
    EXPECT_EQ(0x00, _cpu.registers().C);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB22Test)
{
    // SLA D
    _cpu.registers().D = 137; // 10001001
    _loadAndExecute(0xCB, 0x22);
    EXPECT_EQ(18, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().D = 8; // 00001000
    _loadAndExecute(0xCB, 0x22);
    EXPECT_EQ(16, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0x22);
    EXPECT_EQ(0x00, _cpu.registers().D);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB23Test)
{
    // SLA E
    _cpu.registers().E = 137; // 10001001
    _loadAndExecute(0xCB, 0x23);
    EXPECT_EQ(18, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().E = 8; // 00001000
    _loadAndExecute(0xCB, 0x23);
    EXPECT_EQ(16, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0x23);
    EXPECT_EQ(0x00, _cpu.registers().E);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB24Test)
{
    // SLA H
    _cpu.registers().H = 137; // 10001001
    _loadAndExecute(0xCB, 0x24);
    EXPECT_EQ(18, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().H = 8; // 00001000
    _loadAndExecute(0xCB, 0x24);
    EXPECT_EQ(16, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0x24);
    EXPECT_EQ(0x00, _cpu.registers().H);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB25Test)
{
    // SLA L
    _cpu.registers().L = 137; // 10001001
    _loadAndExecute(0xCB, 0x25);
    EXPECT_EQ(18, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().L = 8; // 00001000
    _loadAndExecute(0xCB, 0x25);
    EXPECT_EQ(16, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0x25);
    EXPECT_EQ(0x00, _cpu.registers().L);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB26Test)
{
    // SLA (HL)
    _mem[_cpu.registers().HL] = 137; // 10001001
    _loadAndExecute(0xCB, 0x26);
    EXPECT_EQ(18, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,1);

    _mem[_cpu.registers().HL] = 8; // 00001000
    _loadAndExecute(0xCB, 0x26);
    EXPECT_EQ(16, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);

    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0x26);
    EXPECT_EQ(0x00, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB27Test)
{
    // SLA A
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0xCB, 0x27);
    EXPECT_EQ(18, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().A = 8; // 00001000
    _loadAndExecute(0xCB, 0x27);
    EXPECT_EQ(16, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0x27);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB28Test)
{
    // SRA B
    _cpu.registers().B = 137; // 10001001
    _loadAndExecute(0xCB, 0x28);
    EXPECT_EQ(196, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().B = 8; // 00001000
    _loadAndExecute(0xCB, 0x28);
    EXPECT_EQ(4, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0x28);
    EXPECT_EQ(0x00, _cpu.registers().B);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB29Test)
{
    // SRA C
    _cpu.registers().C = 137; // 10001001
    _loadAndExecute(0xCB, 0x29);
    EXPECT_EQ(196, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().C = 8; // 00001000
    _loadAndExecute(0xCB, 0x29);
    EXPECT_EQ(4, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0x29);
    EXPECT_EQ(0x00, _cpu.registers().C);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB2ATest)
{
    // SRA D
    _cpu.registers().D = 137; // 10001001
    _loadAndExecute(0xCB, 0x2A);
    EXPECT_EQ(196, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().D = 8; // 00001000
    _loadAndExecute(0xCB, 0x2A);
    EXPECT_EQ(4, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0x2A);
    EXPECT_EQ(0x00, _cpu.registers().D);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB2BTest)
{
    // SRA E
    _cpu.registers().E = 137; // 10001001
    _loadAndExecute(0xCB, 0x2B);
    EXPECT_EQ(196, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().E = 8; // 00001000
    _loadAndExecute(0xCB, 0x2B);
    EXPECT_EQ(4, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0x2B);
    EXPECT_EQ(0x00, _cpu.registers().E);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB2CTest)
{
    // SRA H
    _cpu.registers().H = 137; // 10001001
    _loadAndExecute(0xCB, 0x2C);
    EXPECT_EQ(196, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().H = 8; // 00001000
    _loadAndExecute(0xCB, 0x2C);
    EXPECT_EQ(4, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0x2C);
    EXPECT_EQ(0x00, _cpu.registers().H);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB2DTest)
{
    // SRA L
    _cpu.registers().L = 137; // 10001001
    _loadAndExecute(0xCB, 0x2D);
    EXPECT_EQ(196, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().L = 8; // 00001000
    _loadAndExecute(0xCB, 0x2D);
    EXPECT_EQ(4, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0x2D);
    EXPECT_EQ(0x00, _cpu.registers().L);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB2ETest)
{
    // SRA (HL)
    _mem[_cpu.registers().HL] = 137; // 10001001
    _loadAndExecute(0xCB, 0x2E);
    EXPECT_EQ(196, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,1);

    _mem[_cpu.registers().HL] = 8; // 00001000
    _loadAndExecute(0xCB, 0x2E);
    EXPECT_EQ(4, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);

    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0x2E);
    EXPECT_EQ(0x00, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB2FTest)
{
    // SRA A
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0xCB, 0x2F);
    EXPECT_EQ(196, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().A = 8; // 00001000
    _loadAndExecute(0xCB, 0x2F);
    EXPECT_EQ(4, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0x2F);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB30Test)
{
    // SWAP B
    _cpu.registers().B = 0xFA;
    _loadAndExecute(0xCB, 0x30);
    EXPECT_EQ(0xAF, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0x30);
    EXPECT_EQ(0x00, _cpu.registers().B);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB31Test)
{
    // SWAP C
    _cpu.registers().C = 0xFA;
    _loadAndExecute(0xCB, 0x31);
    EXPECT_EQ(0xAF, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0x31);
    EXPECT_EQ(0x00, _cpu.registers().C);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB32Test)
{
    // SWAP D
    _cpu.registers().D = 0xFA;
    _loadAndExecute(0xCB, 0x32);
    EXPECT_EQ(0xAF, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0x32);
    EXPECT_EQ(0x00, _cpu.registers().D);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB33Test)
{
    // SWAP E
    _cpu.registers().E = 0xFA;
    _loadAndExecute(0xCB, 0x33);
    EXPECT_EQ(0xAF, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0x33);
    EXPECT_EQ(0x00, _cpu.registers().E);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB34Test)
{
    // SWAP H
    _cpu.registers().H = 0xFA;
    _loadAndExecute(0xCB, 0x34);
    EXPECT_EQ(0xAF, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0x34);
    EXPECT_EQ(0x00, _cpu.registers().H);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB35Test)
{
    // SWAP L
    _cpu.registers().L = 0xFA;
    _loadAndExecute(0xCB, 0x35);
    EXPECT_EQ(0xAF, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0x35);
    EXPECT_EQ(0x00, _cpu.registers().L);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB36Test)
{
    // SWAP (HL)
    _mem[_cpu.registers().HL] = 0xFA;
    _loadAndExecute(0xCB, 0x36);
    EXPECT_EQ(0xAF, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);

    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0x36);
    EXPECT_EQ(0x00, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB37Test)
{
    // SWAP A
    _cpu.registers().A = 0xFA;
    _loadAndExecute(0xCB, 0x37);
    EXPECT_EQ(0xAF, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0x37);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB38Test)
{
    // SRL B
    _cpu.registers().B = 137; // 10001001
    _loadAndExecute(0xCB, 0x38);
    EXPECT_EQ(68, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().B = 8; // 00001000
    _loadAndExecute(0xCB, 0x38);
    EXPECT_EQ(4, _cpu.registers().B);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0x38);
    EXPECT_EQ(0x00, _cpu.registers().B);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB39Test)
{
    // SRL C
    _cpu.registers().C = 137; // 10001001
    _loadAndExecute(0xCB, 0x39);
    EXPECT_EQ(68, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().C = 8; // 00001000
    _loadAndExecute(0xCB, 0x39);
    EXPECT_EQ(4, _cpu.registers().C);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0x39);
    EXPECT_EQ(0x00, _cpu.registers().C);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB3ATest)
{
    // SRL D
    _cpu.registers().D = 137; // 10001001
    _loadAndExecute(0xCB, 0x3A);
    EXPECT_EQ(68, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().D = 8; // 00001000
    _loadAndExecute(0xCB, 0x3A);
    EXPECT_EQ(4, _cpu.registers().D);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0x3A);
    EXPECT_EQ(0x00, _cpu.registers().D);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB3BTest)
{
    // SRL E
    _cpu.registers().E = 137; // 10001001
    _loadAndExecute(0xCB, 0x3B);
    EXPECT_EQ(68, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().E = 8; // 00001000
    _loadAndExecute(0xCB, 0x3B);
    EXPECT_EQ(4, _cpu.registers().E);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0x3B);
    EXPECT_EQ(0x00, _cpu.registers().E);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB3CTest)
{
    // SRL H
    _cpu.registers().H = 137; // 10001001
    _loadAndExecute(0xCB, 0x3C);
    EXPECT_EQ(68, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().H = 8; // 00001000
    _loadAndExecute(0xCB, 0x3C);
    EXPECT_EQ(4, _cpu.registers().H);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0x3C);
    EXPECT_EQ(0x00, _cpu.registers().H);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB3DTest)
{
    // SRL L
    _cpu.registers().L = 137; // 10001001
    _loadAndExecute(0xCB, 0x3D);
    EXPECT_EQ(68, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().L = 8; // 00001000
    _loadAndExecute(0xCB, 0x3D);
    EXPECT_EQ(4, _cpu.registers().L);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0x3D);
    EXPECT_EQ(0x00, _cpu.registers().L);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB3ETest)
{
    // SRL (HL)
    _mem[_cpu.registers().HL] = 137; // 10001001
    _loadAndExecute(0xCB, 0x3E);
    EXPECT_EQ(68, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,1);

    _mem[_cpu.registers().HL] = 8; // 00001000
    _loadAndExecute(0xCB, 0x3E);
    EXPECT_EQ(4, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(0,0,0,0);

    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0x3E);
    EXPECT_EQ(0x00, _mem[_cpu.registers().HL]);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB3FTest)
{
    // SRL A
    _cpu.registers().A = 137; // 10001001
    _loadAndExecute(0xCB, 0x3F);
    EXPECT_EQ(68, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,1);

    _cpu.registers().A = 8; // 00001000
    _loadAndExecute(0xCB, 0x3F);
    EXPECT_EQ(4, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0x3F);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xCB40Test)
{
    // BIT 0,B
    _cpu.registers().B = ~0x01;
    _loadAndExecute(0xCB, 0x40);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().B = 0x01;
    _loadAndExecute(0xCB, 0x40);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB41Test)
{
    // BIT 0,C
    _cpu.registers().C = ~0x01;
    _loadAndExecute(0xCB, 0x41);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().C = 0x01;
    _loadAndExecute(0xCB, 0x41);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB42Test)
{
    // BIT 0,D
    _cpu.registers().D = ~0x01;
    _loadAndExecute(0xCB, 0x42);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().D = 0x01;
    _loadAndExecute(0xCB, 0x42);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB43Test)
{
    // BIT 0,E
    _cpu.registers().E = ~0x01;
    _loadAndExecute(0xCB, 0x43);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().E = 0x01;
    _loadAndExecute(0xCB, 0x43);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB44Test)
{
    // BIT 0,H
    _cpu.registers().H = ~0x01;
    _loadAndExecute(0xCB, 0x44);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().H = 0x01;
    _loadAndExecute(0xCB, 0x44);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB45Test)
{
    // BIT 0,L
    _cpu.registers().L = ~0x01;
    _loadAndExecute(0xCB, 0x45);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().L = 0x01;
    _loadAndExecute(0xCB, 0x45);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB46Test)
{
    // BIT 0,(HL)
    _mem[_cpu.registers().HL] = ~0x01;
    _loadAndExecute(0xCB, 0x46);
    EXPECT_FLAGS(1,0,1,0);

    _mem[_cpu.registers().HL] = 0x01;
    _loadAndExecute(0xCB, 0x46);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB47Test)
{
    // BIT 0,A
    _cpu.registers().A = ~0x01;
    _loadAndExecute(0xCB, 0x47);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().A = 0x01;
    _loadAndExecute(0xCB, 0x47);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB48Test)
{
    // BIT 1,B
    _cpu.registers().B = ~0x02;
    _loadAndExecute(0xCB, 0x48);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().B = 0x02;
    _loadAndExecute(0xCB, 0x48);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB49Test)
{
    // BIT 1,C
    _cpu.registers().C = ~0x02;
    _loadAndExecute(0xCB, 0x49);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().C = 0x02;
    _loadAndExecute(0xCB, 0x49);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB4ATest)
{
    // BIT 1,D
    _cpu.registers().D = ~0x02;
    _loadAndExecute(0xCB, 0x4A);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().D = 0x02;
    _loadAndExecute(0xCB, 0x4A);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB4BTest)
{
    // BIT 1,E
    _cpu.registers().E = ~0x02;
    _loadAndExecute(0xCB, 0x4B);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().E = 0x02;
    _loadAndExecute(0xCB, 0x4B);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB4CTest)
{
    // BIT 1,H
    _cpu.registers().H = ~0x02;
    _loadAndExecute(0xCB, 0x4C);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().H = 0x02;
    _loadAndExecute(0xCB, 0x4C);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB4DTest)
{
    // BIT 1,L
    _cpu.registers().L = ~0x02;
    _loadAndExecute(0xCB, 0x4D);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().L = 0x02;
    _loadAndExecute(0xCB, 0x4D);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB4ETest)
{
    // BIT 1,(HL)
    _mem[_cpu.registers().HL] = ~0x02;
    _loadAndExecute(0xCB, 0x4E);
    EXPECT_FLAGS(1,0,1,0);

    _mem[_cpu.registers().HL] = 0x02;
    _loadAndExecute(0xCB, 0x4E);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB4FTest)
{
    // BIT 1,A
    _cpu.registers().A = ~0x02;
    _loadAndExecute(0xCB, 0x4F);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().A = 0x02;
    _loadAndExecute(0xCB, 0x4F);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB50Test)
{
    // BIT 2,B
    _cpu.registers().B = ~0x04;
    _loadAndExecute(0xCB, 0x50);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().B = 0x04;
    _loadAndExecute(0xCB, 0x50);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB51Test)
{
    // BIT 2,C
    _cpu.registers().C = ~0x04;
    _loadAndExecute(0xCB, 0x51);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().C = 0x04;
    _loadAndExecute(0xCB, 0x51);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB52Test)
{
    // BIT 2,D
    _cpu.registers().D = ~0x04;
    _loadAndExecute(0xCB, 0x52);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().D = 0x04;
    _loadAndExecute(0xCB, 0x52);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB53Test)
{
    // BIT 2,E
    _cpu.registers().E = ~0x04;
    _loadAndExecute(0xCB, 0x53);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().E = 0x04;
    _loadAndExecute(0xCB, 0x53);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB54Test)
{
    // BIT 2,H
    _cpu.registers().H = ~0x04;
    _loadAndExecute(0xCB, 0x54);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().H = 0x04;
    _loadAndExecute(0xCB, 0x54);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB55Test)
{
    // BIT 2,L
    _cpu.registers().L = ~0x04;
    _loadAndExecute(0xCB, 0x55);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().L = 0x04;
    _loadAndExecute(0xCB, 0x55);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB56Test)
{
    // BIT 2,(HL)
    _mem[_cpu.registers().HL] = ~0x04;
    _loadAndExecute(0xCB, 0x56);
    EXPECT_FLAGS(1,0,1,0);

    _mem[_cpu.registers().HL] = 0x04;
    _loadAndExecute(0xCB, 0x56);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB57Test)
{
    // BIT 2,A
    _cpu.registers().A = ~0x04;
    _loadAndExecute(0xCB, 0x57);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().A = 0x04;
    _loadAndExecute(0xCB, 0x57);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB58Test)
{
    // BIT 3,B
    _cpu.registers().B = ~0x08;
    _loadAndExecute(0xCB, 0x58);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().B = 0x08;
    _loadAndExecute(0xCB, 0x58);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB59Test)
{
    // BIT 3,C
    _cpu.registers().C = ~0x08;
    _loadAndExecute(0xCB, 0x59);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().C = 0x08;
    _loadAndExecute(0xCB, 0x59);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB5ATest)
{
    // BIT 3,D
    _cpu.registers().D = ~0x08;
    _loadAndExecute(0xCB, 0x5A);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().D = 0x08;
    _loadAndExecute(0xCB, 0x5A);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB5BTest)
{
    // BIT 3,E
    _cpu.registers().E = ~0x08;
    _loadAndExecute(0xCB, 0x5B);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().E = 0x08;
    _loadAndExecute(0xCB, 0x5B);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB5CTest)
{
    // BIT 3,H
    _cpu.registers().H = ~0x08;
    _loadAndExecute(0xCB, 0x5C);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().H = 0x08;
    _loadAndExecute(0xCB, 0x5C);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB5DTest)
{
    // BIT 3,L
    _cpu.registers().L = ~0x08;
    _loadAndExecute(0xCB, 0x5D);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().L = 0x08;
    _loadAndExecute(0xCB, 0x5D);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB5ETest)
{
    // BIT 3,(HL)
    _mem[_cpu.registers().HL] = ~0x08;
    _loadAndExecute(0xCB, 0x5E);
    EXPECT_FLAGS(1,0,1,0);

    _mem[_cpu.registers().HL] = 0x08;
    _loadAndExecute(0xCB, 0x5E);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB5FTest)
{
    // BIT 3,A
    _cpu.registers().A = ~0x08;
    _loadAndExecute(0xCB, 0x5F);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().A = 0x08;
    _loadAndExecute(0xCB, 0x5F);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB60Test)
{
    // BIT 4,B
    _cpu.registers().B = ~0x10;
    _loadAndExecute(0xCB, 0x60);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().B = 0x10;
    _loadAndExecute(0xCB, 0x60);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB61Test)
{
    // BIT 4,C
    _cpu.registers().C = ~0x10;
    _loadAndExecute(0xCB, 0x61);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().C = 0x10;
    _loadAndExecute(0xCB, 0x61);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB62Test)
{
    // BIT 4,D
    _cpu.registers().D = ~0x10;
    _loadAndExecute(0xCB, 0x62);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().D = 0x10;
    _loadAndExecute(0xCB, 0x62);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB63Test)
{
    // BIT 4,E
    _cpu.registers().E = ~0x10;
    _loadAndExecute(0xCB, 0x63);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().E = 0x10;
    _loadAndExecute(0xCB, 0x63);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB64Test)
{
    // BIT 4,H
    _cpu.registers().H = ~0x10;
    _loadAndExecute(0xCB, 0x64);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().H = 0x10;
    _loadAndExecute(0xCB, 0x64);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB65Test)
{
    // BIT 4,L
    _cpu.registers().L = ~0x10;
    _loadAndExecute(0xCB, 0x65);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().L = 0x10;
    _loadAndExecute(0xCB, 0x65);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB66Test)
{
    // BIT 4,(HL)
    _mem[_cpu.registers().HL] = ~0x10;
    _loadAndExecute(0xCB, 0x66);
    EXPECT_FLAGS(1,0,1,0);

    _mem[_cpu.registers().HL] = 0x10;
    _loadAndExecute(0xCB, 0x66);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB67Test)
{
    // BIT 4,A
    _cpu.registers().A = ~0x10;
    _loadAndExecute(0xCB, 0x67);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().A = 0x10;
    _loadAndExecute(0xCB, 0x67);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB68Test)
{
    // BIT 5,B
    _cpu.registers().B = ~0x20;
    _loadAndExecute(0xCB, 0x68);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().B = 0x20;
    _loadAndExecute(0xCB, 0x68);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB69Test)
{
    // BIT 5,C
    _cpu.registers().C = ~0x20;
    _loadAndExecute(0xCB, 0x69);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().C = 0x20;
    _loadAndExecute(0xCB, 0x69);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB6ATest)
{
    // BIT 5,D
    _cpu.registers().D = ~0x20;
    _loadAndExecute(0xCB, 0x6A);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().D = 0x20;
    _loadAndExecute(0xCB, 0x6A);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB6BTest)
{
    // BIT 5,E
    _cpu.registers().E = ~0x20;
    _loadAndExecute(0xCB, 0x6B);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().E = 0x20;
    _loadAndExecute(0xCB, 0x6B);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB6CTest)
{
    // BIT 5,H
    _cpu.registers().H = ~0x20;
    _loadAndExecute(0xCB, 0x6C);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().H = 0x20;
    _loadAndExecute(0xCB, 0x6C);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB6DTest)
{
    // BIT 5,L
    _cpu.registers().L = ~0x20;
    _loadAndExecute(0xCB, 0x6D);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().L = 0x20;
    _loadAndExecute(0xCB, 0x6D);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB6ETest)
{
    // BIT 5,(HL)
    _mem[_cpu.registers().HL] = ~0x20;
    _loadAndExecute(0xCB, 0x6E);
    EXPECT_FLAGS(1,0,1,0);

    _mem[_cpu.registers().HL] = 0x20;
    _loadAndExecute(0xCB, 0x6E);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB6FTest)
{
    // BIT 5,A
    _cpu.registers().A = ~0x20;
    _loadAndExecute(0xCB, 0x6F);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().A = 0x20;
    _loadAndExecute(0xCB, 0x6F);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB70Test)
{
    // BIT 6,B
    _cpu.registers().B = ~0x40;
    _loadAndExecute(0xCB, 0x70);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().B = 0x40;
    _loadAndExecute(0xCB, 0x70);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB71Test)
{
    // BIT 6,C
    _cpu.registers().C = ~0x40;
    _loadAndExecute(0xCB, 0x71);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().C = 0x40;
    _loadAndExecute(0xCB, 0x71);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB72Test)
{
    // BIT 6,D
    _cpu.registers().D = ~0x40;
    _loadAndExecute(0xCB, 0x72);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().D = 0x40;
    _loadAndExecute(0xCB, 0x72);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB73Test)
{
    // BIT 6,E
    _cpu.registers().E = ~0x40;
    _loadAndExecute(0xCB, 0x73);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().E = 0x40;
    _loadAndExecute(0xCB, 0x73);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB74Test)
{
    // BIT 6,H
    _cpu.registers().H = ~0x40;
    _loadAndExecute(0xCB, 0x74);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().H = 0x40;
    _loadAndExecute(0xCB, 0x74);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB75Test)
{
    // BIT 6,L
    _cpu.registers().L = ~0x40;
    _loadAndExecute(0xCB, 0x75);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().L = 0x40;
    _loadAndExecute(0xCB, 0x75);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB76Test)
{
    // BIT 6,(HL)
    _mem[_cpu.registers().HL] = ~0x40;
    _loadAndExecute(0xCB, 0x76);
    EXPECT_FLAGS(1,0,1,0);

    _mem[_cpu.registers().HL] = 0x40;
    _loadAndExecute(0xCB, 0x76);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB77Test)
{
    // BIT 6,A
    _cpu.registers().A = ~0x40;
    _loadAndExecute(0xCB, 0x77);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().A = 0x40;
    _loadAndExecute(0xCB, 0x77);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB78Test)
{
    // BIT 7,B
    _cpu.registers().B = static_cast<gb::Word>(~0x80);
    _loadAndExecute(0xCB, 0x78);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().B = 0x80;
    _loadAndExecute(0xCB, 0x78);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB79Test)
{
    // BIT 7,C
    _cpu.registers().C = static_cast<gb::Word>(~0x80);
    _loadAndExecute(0xCB, 0x78);
    _loadAndExecute(0xCB, 0x79);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().C = 0x80;
    _loadAndExecute(0xCB, 0x79);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB7ATest)
{
    // BIT 7,D
    _cpu.registers().D = static_cast<gb::Word>(~0x80);
    _loadAndExecute(0xCB, 0x7A);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().D = 0x80;
    _loadAndExecute(0xCB, 0x7A);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB7BTest)
{
    // BIT 7,E
    _cpu.registers().E = static_cast<gb::Word>(~0x80);
    _loadAndExecute(0xCB, 0x7B);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().E = 0x80;
    _loadAndExecute(0xCB, 0x7B);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB7CTest)
{
    // BIT 7,H
    _cpu.registers().H = static_cast<gb::Word>(~0x80);
    _loadAndExecute(0xCB, 0x7C);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().H = 0x80;
    _loadAndExecute(0xCB, 0x7C);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB7DTest)
{
    // BIT 7,L
    _cpu.registers().L = static_cast<gb::Word>(~0x80);
    _loadAndExecute(0xCB, 0x7D);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().L = 0x80;
    _loadAndExecute(0xCB, 0x7D);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB7ETest)
{
    // BIT 7,(HL)
    _mem[_cpu.registers().HL] = static_cast<gb::Word>(~0x80);
    _loadAndExecute(0xCB, 0x7E);
    EXPECT_FLAGS(1,0,1,0);

    _mem[_cpu.registers().HL] = static_cast<gb::Word>(0x80);
    _loadAndExecute(0xCB, 0x7E);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB7FTest)
{
    // BIT 7,A
    _cpu.registers().A = static_cast<gb::Word>(~0x80);
    _loadAndExecute(0xCB, 0x7F);
    EXPECT_FLAGS(1,0,1,0);

    _cpu.registers().A = 0x80;
    _loadAndExecute(0xCB, 0x7F);
    EXPECT_FLAGS(0,0,1,0);
}

TEST_F(CpuTest, Opcode0xCB80Test)
{
    // RES 0,B
    _cpu.registers().B = 0xFF;
    _loadAndExecute(0xCB, 0x80);
    EXPECT_EQ(0xFE, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCB81Test)
{
    // RES 0,C
    _cpu.registers().C = 0xFF;
    _loadAndExecute(0xCB, 0x81);
    EXPECT_EQ(0xFE, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCB82Test)
{
    // RES 0,D
    _cpu.registers().D = 0xFF;
    _loadAndExecute(0xCB, 0x82);
    EXPECT_EQ(0xFE, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCB83Test)
{
    // RES 0,E
    _cpu.registers().E = 0xFF;
    _loadAndExecute(0xCB, 0x83);
    EXPECT_EQ(0xFE, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCB84Test)
{
    // RES 0,H
    _cpu.registers().H = 0xFF;
    _loadAndExecute(0xCB, 0x84);
    EXPECT_EQ(0xFE, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCB85Test)
{
    // RES 0,L
    _cpu.registers().L = 0xFF;
    _loadAndExecute(0xCB, 0x85);
    EXPECT_EQ(0xFE, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCB86Test)
{
    // RES 0,(HL)
    _mem[_cpu.registers().HL] = 0xFF;
    _loadAndExecute(0xCB, 0x86);
    EXPECT_EQ(0xFE, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCB87Test)
{
    // RES 0,A
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xCB, 0x87);
    EXPECT_EQ(0xFE, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCB88Test)
{
    // RES 1,B
    _cpu.registers().B = 0xFF;
    _loadAndExecute(0xCB, 0x88);
    EXPECT_EQ(0xFD, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCB89Test)
{
    // RES 1,C
    _cpu.registers().C = 0xFF;
    _loadAndExecute(0xCB, 0x89);
    EXPECT_EQ(0xFD, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCB8ATest)
{
    // RES 1,D
    _cpu.registers().D = 0xFF;
    _loadAndExecute(0xCB, 0x8A);
    EXPECT_EQ(0xFD, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCB8BTest)
{
    // RES 1,E
    _cpu.registers().E = 0xFF;
    _loadAndExecute(0xCB, 0x8B);
    EXPECT_EQ(0xFD, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCB8CTest)
{
    // RES 1,H
    _cpu.registers().H = 0xFF;
    _loadAndExecute(0xCB, 0x8C);
    EXPECT_EQ(0xFD, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCB8DTest)
{
    // RES 1,L
    _cpu.registers().L = 0xFF;
    _loadAndExecute(0xCB, 0x8D);
    EXPECT_EQ(0xFD, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCB8ETest)
{
    // RES 1,(HL)
    _mem[_cpu.registers().HL] = 0xFF;
    _loadAndExecute(0xCB, 0x8E);
    EXPECT_EQ(0xFD, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCB8FTest)
{
    // RES 1,A
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xCB, 0x8F);
    EXPECT_EQ(0xFD, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCB90Test)
{
    // RES 2,B
    _cpu.registers().B = 0xFF;
    _loadAndExecute(0xCB, 0x90);
    EXPECT_EQ(0xFB, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCB91Test)
{
    // RES 2,C
    _cpu.registers().C = 0xFF;
    _loadAndExecute(0xCB, 0x91);
    EXPECT_EQ(0xFB, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCB92Test)
{
    // RES 2,D
    _cpu.registers().D = 0xFF;
    _loadAndExecute(0xCB, 0x92);
    EXPECT_EQ(0xFB, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCB93Test)
{
    // RES 2,E
    _cpu.registers().E = 0xFF;
    _loadAndExecute(0xCB, 0x93);
    EXPECT_EQ(0xFB, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCB94Test)
{
    // RES 2,H
    _cpu.registers().H = 0xFF;
    _loadAndExecute(0xCB, 0x94);
    EXPECT_EQ(0xFB, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCB95Test)
{
    // RES 2,L
    _cpu.registers().L = 0xFF;
    _loadAndExecute(0xCB, 0x95);
    EXPECT_EQ(0xFB, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCB96Test)
{
    // RES 2,(HL)
    _mem[_cpu.registers().HL] = 0xFF;
    _loadAndExecute(0xCB, 0x96);
    EXPECT_EQ(0xFB, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCB97Test)
{
    // RES 2,A
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xCB, 0x97);
    EXPECT_EQ(0xFB, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCB98Test)
{
    // RES 3,B
    _cpu.registers().B = 0xFF;
    _loadAndExecute(0xCB, 0x98);
    EXPECT_EQ(0xF7, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCB99Test)
{
    // RES 3,C
    _cpu.registers().C = 0xFF;
    _loadAndExecute(0xCB, 0x99);
    EXPECT_EQ(0xF7, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCB9ATest)
{
    // RES 3,D
    _cpu.registers().D = 0xFF;
    _loadAndExecute(0xCB, 0x9A);
    EXPECT_EQ(0xF7, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCB9BTest)
{
    // RES 3,E
    _cpu.registers().E = 0xFF;
    _loadAndExecute(0xCB, 0x9B);
    EXPECT_EQ(0xF7, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCB9CTest)
{
    // RES 3,H
    _cpu.registers().H = 0xFF;
    _loadAndExecute(0xCB, 0x9C);
    EXPECT_EQ(0xF7, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCB9DTest)
{
    // RES 3,L
    _cpu.registers().L = 0xFF;
    _loadAndExecute(0xCB, 0x9D);
    EXPECT_EQ(0xF7, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCB9ETest)
{
    // RES 3,(HL)
    _mem[_cpu.registers().HL] = 0xFF;
    _loadAndExecute(0xCB, 0x9E);
    EXPECT_EQ(0xF7, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCB9FTest)
{
    // RES 3,A
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xCB, 0x9F);
    EXPECT_EQ(0xF7, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCBA0Test)
{
    // RES 4,B
    _cpu.registers().B = 0xFF;
    _loadAndExecute(0xCB, 0xA0);
    EXPECT_EQ(0xEF, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCBA1Test)
{
    // RES 4,C
    _cpu.registers().C = 0xFF;
    _loadAndExecute(0xCB, 0xA1);
    EXPECT_EQ(0xEF, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCBA2Test)
{
    // RES 4,D
    _cpu.registers().D = 0xFF;
    _loadAndExecute(0xCB, 0xA2);
    EXPECT_EQ(0xEF, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCBA3Test)
{
    // RES 4,E
    _cpu.registers().E = 0xFF;
    _loadAndExecute(0xCB, 0xA3);
    EXPECT_EQ(0xEF, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCBA4Test)
{
    // RES 4,H
    _cpu.registers().H = 0xFF;
    _loadAndExecute(0xCB, 0xA4);
    EXPECT_EQ(0xEF, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCBA5Test)
{
    // RES 4,L
    _cpu.registers().L = 0xFF;
    _loadAndExecute(0xCB, 0xA5);
    EXPECT_EQ(0xEF, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCBA6Test)
{
    // RES 4,(HL)
    _mem[_cpu.registers().HL] = 0xFF;
    _loadAndExecute(0xCB, 0xA6);
    EXPECT_EQ(0xEF, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCBA7Test)
{
    // RES 4,A
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xCB, 0xA7);
    EXPECT_EQ(0xEF, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCBA8Test)
{
    // RES 5,B
    _cpu.registers().B = 0xFF;
    _loadAndExecute(0xCB, 0xA8);
    EXPECT_EQ(0xDF, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCBA9Test)
{
    // RES 5,C
    _cpu.registers().C = 0xFF;
    _loadAndExecute(0xCB, 0xA9);
    EXPECT_EQ(0xDF, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCBAATest)
{
    // RES 5,D
    _cpu.registers().D = 0xFF;
    _loadAndExecute(0xCB, 0xAA);
    EXPECT_EQ(0xDF, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCBABTest)
{
    // RES 5,E
    _cpu.registers().E = 0xFF;
    _loadAndExecute(0xCB, 0xAB);
    EXPECT_EQ(0xDF, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCBACTest)
{
    // RES 5,H
    _cpu.registers().H = 0xFF;
    _loadAndExecute(0xCB, 0xAC);
    EXPECT_EQ(0xDF, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCBADTest)
{
    // RES 5,L
    _cpu.registers().L = 0xFF;
    _loadAndExecute(0xCB, 0xAD);
    EXPECT_EQ(0xDF, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCBAETest)
{
    // RES 5,(HL)
    _mem[_cpu.registers().HL] = 0xFF;
    _loadAndExecute(0xCB, 0xAE);
    EXPECT_EQ(0xDF, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCBAFTest)
{
    // RES 5,A
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xCB, 0xAF);
    EXPECT_EQ(0xDF, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCBB0Test)
{
    // RES 6,B
    _cpu.registers().B = 0xFF;
    _loadAndExecute(0xCB, 0xA8);
    EXPECT_EQ(0xDF, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCBB1Test)
{
    // RES 6,C
    _cpu.registers().C = 0xFF;
    _loadAndExecute(0xCB, 0xB1);
    EXPECT_EQ(0xBF, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCBB2Test)
{
    // RES 6,D
    _cpu.registers().D = 0xFF;
    _loadAndExecute(0xCB, 0xB2);
    EXPECT_EQ(0xBF, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCBB3Test)
{
    // RES 6,E
    _cpu.registers().E = 0xFF;
    _loadAndExecute(0xCB, 0xB3);
    EXPECT_EQ(0xBF, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCBB4Test)
{
    // RES 6,H
    _cpu.registers().H = 0xFF;
    _loadAndExecute(0xCB, 0xB4);
    EXPECT_EQ(0xBF, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCBB5Test)
{
    // RES 6,L
    _cpu.registers().L = 0xFF;
    _loadAndExecute(0xCB, 0xB5);
    EXPECT_EQ(0xBF, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCBB6Test)
{
    // RES 6,(HL)
    _mem[_cpu.registers().HL] = 0xFF;
    _loadAndExecute(0xCB, 0xB6);
    EXPECT_EQ(0xBF, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCBB7Test)
{
    // RES 6,A
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xCB, 0xB7);
    EXPECT_EQ(0xBF, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCBB8Test)
{
    // RES 7,B
    _cpu.registers().B = 0xFF;
    _loadAndExecute(0xCB, 0xB8);
    EXPECT_EQ(0x7F, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCBB9Test)
{
    // RES 7,C
    _cpu.registers().C = 0xFF;
    _loadAndExecute(0xCB, 0xB9);
    EXPECT_EQ(0x7F, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCBBATest)
{
    // RES 7,D
    _cpu.registers().D = 0xFF;
    _loadAndExecute(0xCB, 0xBA);
    EXPECT_EQ(0x7F, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCBBBTest)
{
    // RES 7,E
    _cpu.registers().E = 0xFF;
    _loadAndExecute(0xCB, 0xBB);
    EXPECT_EQ(0x7F, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCBBCTest)
{
    // RES 7,H
    _cpu.registers().H = 0xFF;
    _loadAndExecute(0xCB, 0xBC);
    EXPECT_EQ(0x7F, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCBBDTest)
{
    // RES 7,L
    _cpu.registers().L = 0xFF;
    _loadAndExecute(0xCB, 0xBD);
    EXPECT_EQ(0x7F, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCBBETest)
{
    // RES 7,(HL)
    _mem[_cpu.registers().HL] = 0xFF;
    _loadAndExecute(0xCB, 0xBE);
    EXPECT_EQ(0x7F, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCBBFTest)
{
    // RES 7,A
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xCB, 0xBF);
    EXPECT_EQ(0x7F, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCBC0Test)
{
    // SET 0,B
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0xC0);
    EXPECT_EQ(0x01, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCBC1Test)
{
    // SET 0,C
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0xC1);
    EXPECT_EQ(0x01, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCBC2Test)
{
    // SET 0,D
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0xC2);
    EXPECT_EQ(0x01, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCBC3Test)
{
    // SET 0,E
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0xC3);
    EXPECT_EQ(0x01, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCBC4Test)
{
    // SET 0,H
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0xC4);
    EXPECT_EQ(0x01, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCBC5Test)
{
    // SET 0,L
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0xC5);
    EXPECT_EQ(0x01, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCBC6Test)
{
    // SET 0,(HL)
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0xC6);
    EXPECT_EQ(0x01, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCBC7Test)
{
    // SET 0,A
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0xC7);
    EXPECT_EQ(0x01, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCBC8Test)
{
    // SET 1,B
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0xC8);
    EXPECT_EQ(0x02, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCBC9Test)
{
    // SET 1,C
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0xC9);
    EXPECT_EQ(0x02, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCBCATest)
{
    // SET 1,D
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0xCA);
    EXPECT_EQ(0x02, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCBCBTest)
{
    // SET 1,E
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0xCB);
    EXPECT_EQ(0x02, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCBCCTest)
{
    // SET 1,H
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0xCC);
    EXPECT_EQ(0x02, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCBCDTest)
{
    // SET 1,L
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0xCD);
    EXPECT_EQ(0x02, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCBCETest)
{
    // SET 1,(HL)
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0xCE);
    EXPECT_EQ(0x02, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCBCFTest)
{
    // SET 1,A
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0xCF);
    EXPECT_EQ(0x02, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCBD0Test)
{
    // SET 2,B
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0xD0);
    EXPECT_EQ(0x04, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCBD1Test)
{
    // SET 2,C
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0xD1);
    EXPECT_EQ(0x04, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCBD2Test)
{
    // SET 2,D
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0xD2);
    EXPECT_EQ(0x04, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCBD3Test)
{
    // SET 2,E
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0xD3);
    EXPECT_EQ(0x04, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCBD4Test)
{
    // SET 2,H
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0xD4);
    EXPECT_EQ(0x04, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCBD5Test)
{
    // SET 2,L
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0xD5);
    EXPECT_EQ(0x04, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCBD6Test)
{
    // SET 2,(HL)
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0xD6);
    EXPECT_EQ(0x04, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCBD7Test)
{
    // SET 2,A
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0xD7);
    EXPECT_EQ(0x04, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCBD8Test)
{
    // SET 3,B
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0xD8);
    EXPECT_EQ(0x08, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCBD9Test)
{
    // SET 3,C
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0xD9);
    EXPECT_EQ(0x08, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCBDATest)
{
    // SET 3,D
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0xDA);
    EXPECT_EQ(0x08, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCBDBTest)
{
    // SET 3,E
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0xDB);
    EXPECT_EQ(0x08, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCBDCTest)
{
    // SET 3,H
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0xDC);
    EXPECT_EQ(0x08, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCBDDTest)
{
    // SET 3,L
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0xDD);
    EXPECT_EQ(0x08, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCBDETest)
{
    // SET 3,(HL)
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0xDE);
    EXPECT_EQ(0x08, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCBDFTest)
{
    // SET 3,A
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0xDF);
    EXPECT_EQ(0x08, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCBE0Test)
{
    // SET 4,B
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0xE0);
    EXPECT_EQ(0x10, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCBE1Test)
{
    // SET 4,C
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0xE1);
    EXPECT_EQ(0x10, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCBE2Test)
{
    // SET 4,D
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0xE2);
    EXPECT_EQ(0x10, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCBE3Test)
{
    // SET 4,E
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0xE3);
    EXPECT_EQ(0x10, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCBE4Test)
{
    // SET 4,H
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0xE4);
    EXPECT_EQ(0x10, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCBE5Test)
{
    // SET 4,L
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0xE5);
    EXPECT_EQ(0x10, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCBE6Test)
{
    // SET 4,(HL)
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0xE6);
    EXPECT_EQ(0x10, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCBE7Test)
{
    // SET 4,A
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0xE7);
    EXPECT_EQ(0x10, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCBE8Test)
{
    // SET 5,B
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0xE8);
    EXPECT_EQ(0x20, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCBE9Test)
{
    // SET 5,C
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0xE9);
    EXPECT_EQ(0x20, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCBEATest)
{
    // SET 5,D
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0xEA);
    EXPECT_EQ(0x20, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCBEBTest)
{
    // SET 5,E
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0xEB);
    EXPECT_EQ(0x20, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCBECTest)
{
    // SET 5,H
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0xEC);
    EXPECT_EQ(0x20, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCBEDTest)
{
    // SET 5,L
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0xED);
    EXPECT_EQ(0x20, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCBEETest)
{
    // SET 5,(HL)
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0xEE);
    EXPECT_EQ(0x20, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCBEFTest)
{
    // SET 5,A
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0xEF);
    EXPECT_EQ(0x20, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCBF0Test)
{
    // SET 6,B
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0xF0);
    EXPECT_EQ(0x40, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCBF1Test)
{
    // SET 6,C
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0xF1);
    EXPECT_EQ(0x40, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCBF2Test)
{
    // SET 6,D
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0xF2);
    EXPECT_EQ(0x40, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCBF3Test)
{
    // SET 6,E
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0xF3);
    EXPECT_EQ(0x40, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCBF4Test)
{
    // SET 6,H
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0xF4);
    EXPECT_EQ(0x40, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCBF5Test)
{
    // SET 6,L
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0xF5);
    EXPECT_EQ(0x40, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCBF6Test)
{
    // SET 6,(HL)
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0xF6);
    EXPECT_EQ(0x40, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCBF7Test)
{
    // SET 6,A
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0xF7);
    EXPECT_EQ(0x40, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCBF8Test)
{
    // SET 7,B
    _cpu.registers().B = 0x00;
    _loadAndExecute(0xCB, 0xF8);
    EXPECT_EQ(0x80, _cpu.registers().B);
}

TEST_F(CpuTest, Opcode0xCBF9Test)
{
    // SET 7,C
    _cpu.registers().C = 0x00;
    _loadAndExecute(0xCB, 0xF9);
    EXPECT_EQ(0x80, _cpu.registers().C);
}

TEST_F(CpuTest, Opcode0xCBFATest)
{
    // SET 7,D
    _cpu.registers().D = 0x00;
    _loadAndExecute(0xCB, 0xFA);
    EXPECT_EQ(0x80, _cpu.registers().D);
}

TEST_F(CpuTest, Opcode0xCBFBTest)
{
    // SET 7,E
    _cpu.registers().E = 0x00;
    _loadAndExecute(0xCB, 0xFB);
    EXPECT_EQ(0x80, _cpu.registers().E);
}

TEST_F(CpuTest, Opcode0xCBFCTest)
{
    // SET 7,H
    _cpu.registers().H = 0x00;
    _loadAndExecute(0xCB, 0xFC);
    EXPECT_EQ(0x80, _cpu.registers().H);
}

TEST_F(CpuTest, Opcode0xCBFDTest)
{
    // SET 7,L
    _cpu.registers().L = 0x00;
    _loadAndExecute(0xCB, 0xFD);
    EXPECT_EQ(0x80, _cpu.registers().L);
}

TEST_F(CpuTest, Opcode0xCBFETest)
{
    // SET 7,(HL)
    _mem[_cpu.registers().HL] = 0x00;
    _loadAndExecute(0xCB, 0xFE);
    EXPECT_EQ(0x80, _mem[_cpu.registers().HL]);
}

TEST_F(CpuTest, Opcode0xCBFFTest)
{
    // SET 7,A
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xCB, 0xFF);
    EXPECT_EQ(0x80, _cpu.registers().A);
}

TEST_F(CpuTest, Opcode0xCCTest)
{
    // CALL Z,(nn)
    _cpu.registers().F = 0xFF;
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xCC, 0xAA, 0x22);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x22AA, _cpu.registers().PC);
    EXPECT_EQ(0x11, _mem[0xFEFF]);
    EXPECT_EQ(0xAD, _mem[0xFEFE]);
    EXPECT_FLAGS(1,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xCC, 0xAA, 0x22);
    EXPECT_EQ(0xFF00, _cpu.registers().SP);
    EXPECT_EQ(0x11AD, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xCDTest)
{
    // CALL (nn)
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xCD, 0xAA, 0x22);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x22AA, _cpu.registers().PC);
    EXPECT_EQ(0x11, _mem[0xFEFF]);
    EXPECT_EQ(0xAD, _mem[0xFEFE]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xCETest)
{
    // ADC A,n
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x07;
    _loadAndExecute(0xCE, 0xF0);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x0F;
    _loadAndExecute(0xCE, 0x01);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xCE, 0x01);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x06;
    _loadAndExecute(0xCE, 0xF0);
    EXPECT_EQ(0xF7, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x0E;
    _loadAndExecute(0xCE, 0x01);
    EXPECT_EQ(0x10, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFE;
    _loadAndExecute(0xCE, 0x01);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,1);
}

TEST_F(CpuTest, Opcode0xCFTest)
{
    // RST 8H
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xCF);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x0008, _cpu.registers().PC);
    EXPECT_EQ(0x11, (int)_mem[0xFEFF]);
    EXPECT_EQ(0xAB, (int)_mem[0xFEFE]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xD0Test)
{
    // RET NC
    _cpu.registers().F = 0x00;
    _cpu.registers().SP = 0xFF00;
    _mem[0xFF00] = 0xAA;
    _mem[0xFF01] = 0xFF;
    _loadAndExecute(0xD0);
    EXPECT_EQ(0xFFAA, _cpu.registers().PC);
    EXPECT_EQ(0xFF02, _cpu.registers().SP);

    _cpu.registers().PC = 0x0000;

    _cpu.registers().F = 0xFF;
    _cpu.registers().SP = 0xFF00;
    _mem[0xFF00] = 0xAA;
    _mem[0xFF01] = 0xFF;
    _loadAndExecute(0xD0);
    EXPECT_EQ(0x0001, _cpu.registers().PC);
    EXPECT_EQ(0xFF00, _cpu.registers().SP);
}

TEST_F(CpuTest, Opcode0xD1Test)
{
    // POP DE
    _cpu.registers().SP = 0x0038;
    _mem[0x0039] = 0xAB;
    _mem[0x0038] = 0xFE;
    _loadAndExecute(0xD1);
    EXPECT_EQ(0xAB, _cpu.registers().D);
    EXPECT_EQ(0xFE, _cpu.registers().E);
    EXPECT_EQ(_cpu.registers().SP, 0x003A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xD2Test)
{
    // JP NC,(nn)
    _cpu.registers().F = 0x00;
    _loadAndExecute(0xD2, 0xFF, 0xAA);
    EXPECT_EQ(0xAAFF, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().PC = 0x0000;

    _cpu.registers().F = 0xFF;
    _loadAndExecute(0xD2, 0xFF, 0xAA);
    EXPECT_EQ(0x0003, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);
}

TEST_F(CpuTest, Opcode0xD4Test)
{
    // CALL NC,(nn)
    _cpu.registers().F = 0x00;
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xD4, 0xAA, 0x22);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x22AA, _cpu.registers().PC);
    EXPECT_EQ(0x11, _mem[0xFEFF]);
    EXPECT_EQ(0xAD, _mem[0xFEFE]);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xD4, 0xAA, 0x22);
    EXPECT_EQ(0xFF00, _cpu.registers().SP);
    EXPECT_EQ(0x11AD, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);
}

TEST_F(CpuTest, Opcode0xD5Test)
{
    // PUSH DE
    _cpu.registers().SP = 0x003A;
    _cpu.registers().DE = 0xABFE;
    _loadAndExecute(0xD5);
    EXPECT_EQ(_cpu.registers().D, _mem[0x0039]);
    EXPECT_EQ(_cpu.registers().E, _mem[0x0038]);
    EXPECT_EQ(_cpu.registers().SP, 0x0038);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xD6Test)
{
    // SUB n
    _cpu.registers().A = 0x0F;
    _loadAndExecute(0xD6, 0x02);
    EXPECT_EQ(0x0D, _cpu.registers().A);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _loadAndExecute(0xD6, 0x01);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _loadAndExecute(0xD6, 0x01);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _loadAndExecute(0xD6, 0xF1);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().A = 0x05;
    _loadAndExecute(0xD6, 0x05);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0xD7Test)
{
    // RST 10H
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xD7);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x0010, _cpu.registers().PC);
    EXPECT_EQ(0x11, (int)_mem[0xFEFF]);
    EXPECT_EQ(0xAB, (int)_mem[0xFEFE]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xD8Test)
{
    // RET C
    _cpu.registers().F = 0xFF;
    _cpu.registers().SP = 0xFF00;
    _mem[0xFF00] = 0xAA;
    _mem[0xFF01] = 0xFF;
    _loadAndExecute(0xD8);
    EXPECT_EQ(0xFFAA, _cpu.registers().PC);
    EXPECT_EQ(0xFF02, _cpu.registers().SP);

    _cpu.registers().PC = 0x0000;

    _cpu.registers().F = 0x00;
    _cpu.registers().SP = 0xFF00;
    _mem[0xFF00] = 0xAA;
    _mem[0xFF01] = 0xFF;
    _loadAndExecute(0xD8);
    EXPECT_EQ(0x0001, _cpu.registers().PC);
    EXPECT_EQ(0xFF00, _cpu.registers().SP);
}

TEST_F(CpuTest, Opcode0xD9Test)
{
    // RETI
    _cpu.registers().SP = 0xFF00;
    _mem[0xFF00] = 0xAA;
    _mem[0xFF01] = 0xFF;

    EXPECT_FALSE(_cpu.interruptsEnabled());
    _loadAndExecute(0xD9);
    EXPECT_TRUE(_cpu.interruptsEnabled());

    EXPECT_EQ(0xFFAA, _cpu.registers().PC);
    EXPECT_EQ(0xFF02, _cpu.registers().SP);
}

TEST_F(CpuTest, Opcode0xDATest)
{
    // JP C,(nn)
    _cpu.registers().F = 0xFF;
    _loadAndExecute(0xDA, 0xFF, 0xAA);
    EXPECT_EQ(0xAAFF, _cpu.registers().PC);
    EXPECT_FLAGS(1,1,1,1);

    _cpu.registers().PC = 0x0000;

    _cpu.registers().F = 0x00;
    _loadAndExecute(0xDA, 0xFF, 0xAA);
    EXPECT_EQ(0x0003, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xDCTest)
{
    // CALL C,(nn)
    _cpu.registers().F = 0xFF;
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xDC, 0xAA, 0x22);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x22AA, _cpu.registers().PC);
    EXPECT_EQ(0x11, _mem[0xFEFF]);
    EXPECT_EQ(0xAD, _mem[0xFEFE]);
    EXPECT_FLAGS(1,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xDC, 0xAA, 0x22);
    EXPECT_EQ(0xFF00, _cpu.registers().SP);
    EXPECT_EQ(0x11AD, _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xDETest)
{
    // SBC A,n
    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xF0;
    _loadAndExecute(0xDE, 0x07);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x10;
    _loadAndExecute(0xDE, 0x01);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0x00;
    _loadAndExecute(0xDE, 0x01);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0x00;
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xDE, 0xFF);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xF0;
    _loadAndExecute(0xDE, 0x06);
    EXPECT_EQ(0xE9, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x11;
    _loadAndExecute(0xDE, 0x01);
    EXPECT_EQ(0x0F, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0x02;
    _loadAndExecute(0xDE, 0x02);
    EXPECT_EQ(0xFF, _cpu.registers().A);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().F = 0xFF;
    _cpu.registers().A = 0xFF;
    _loadAndExecute(0xDE, 0xFE);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0xDFTest)
{
    // RST 18H
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xDF);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x0018, _cpu.registers().PC);
    EXPECT_EQ(0x11, (int)_mem[0xFEFF]);
    EXPECT_EQ(0xAB, (int)_mem[0xFEFE]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xE0Test)
{
    // LD (FF00+n),A 
    gb::Word val = _cpu.registers().A;
    _loadAndExecute(0xE0, 11);
    EXPECT_EQ(val, _mem[0xFF00 + 11]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xE1Test)
{
    // POP HL
    _cpu.registers().SP = 0x0038;
    _mem[0x0039] = 0xAB;
    _mem[0x0038] = 0xFE;
    _loadAndExecute(0xE1);
    EXPECT_EQ(0xAB, _cpu.registers().H);
    EXPECT_EQ(0xFE, _cpu.registers().L);
    EXPECT_EQ(_cpu.registers().SP, 0x003A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xE2Test)
{
    // LD (FF00+C),A
    gb::Word val = _cpu.registers().A;
    _loadAndExecute(0xE2);
    EXPECT_EQ(val, _mem[0xFF00 + _cpu.registers().C]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xE5Test)
{
    // PUSH HL
    _cpu.registers().SP = 0x003A;
    _cpu.registers().HL = 0xABFE;
    _loadAndExecute(0xE5);
    EXPECT_EQ(_cpu.registers().H, _mem[0x0039]);
    EXPECT_EQ(_cpu.registers().L, _mem[0x0038]);
    EXPECT_EQ(_cpu.registers().SP, 0x0038);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xE6Test)
{
    // AND n
    _cpu.registers().A = 0x0C;
    _loadAndExecute(0xE6, 0x18);
    EXPECT_EQ(0x08, _cpu.registers().A);
    EXPECT_FLAGS(0,0,1,0);

    _cpu.registers().A = 0xF0;
    _loadAndExecute(0xE6, 0x0F);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,1,0);
}

TEST_F(CpuTest, Opcode0xE7Test)
{
    // RST 20H
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xE7);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x0020, _cpu.registers().PC);
    EXPECT_EQ(0x11, (int)_mem[0xFEFF]);
    EXPECT_EQ(0xAB, (int)_mem[0xFEFE]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xE8Test)
{
    // ADD SP,dd
    _cpu.registers().SP = 0x0A80;
    
    gb::Dword val = _cpu.registers().SP + 38;
    _loadAndExecute(0xE8, 38);
    EXPECT_EQ(val, _cpu.registers().SP);
    EXPECT_FLAGS(0,0,0,0);

    val = _cpu.registers().SP - 100;
    _loadAndExecute(0xE8, gb::toSigned8(-100));
    EXPECT_EQ(val, _cpu.registers().SP);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().SP = 0x0F0F;

    val = _cpu.registers().SP + gb::toSigned8(0x11);
    _loadAndExecute(0xE8, gb::toSigned8(0x11));
    EXPECT_EQ(val, _cpu.registers().SP);
    EXPECT_FLAGS(0,0,0,0);

    // Not a half carry -- only applies to highest byte
    _cpu.registers().SP = 0x000F;

    val = _cpu.registers().SP + gb::toSigned8(1);
    _loadAndExecute(0xE8, gb::toSigned8(1));
    EXPECT_EQ(val, _cpu.registers().SP);
    EXPECT_FLAGS(0,0,0,0);

    // Full carry
    _cpu.registers().SP = 0xFFFF;

    val = _cpu.registers().SP + gb::toSigned8(10);
    _loadAndExecute(0xE8, gb::toSigned8(10));
    EXPECT_EQ(val, _cpu.registers().SP);
    EXPECT_FLAGS(0,0,1,1);

    // Underflow carry
    _cpu.registers().SP = 0x000A;

    val = _cpu.registers().SP - 12;
    _loadAndExecute(0xE8, gb::toSigned8(-12));
    EXPECT_EQ(val, _cpu.registers().SP);
    EXPECT_FLAGS(0,0,1,1);
}

TEST_F(CpuTest, Opcode0xE9Test)
{
    // JP (HL)
    _loadAndExecute(0xE9);
    EXPECT_EQ(_mem[_cpu.registers().HL], _cpu.registers().PC);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xEATest)
{
    // LD (nn),A
    gb::Word val = _cpu.registers().A;
    _loadAndExecute(0xEA, 0xFF, 0x01);
    EXPECT_EQ(val, _mem[0x01FF]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xEETest)
{
    // XOR n
    _cpu.registers().A = 0x0C;
    _loadAndExecute(0xEE, 0x18);
    EXPECT_EQ(0x14, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0xF0;
    _loadAndExecute(0xEE, 0xF0);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xEFTest)
{
    // RST 28H
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xEF);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x0028, _cpu.registers().PC);
    EXPECT_EQ(0x11, (int)_mem[0xFEFF]);
    EXPECT_EQ(0xAB, (int)_mem[0xFEFE]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xF0Test)
{
    // LD A,(FF00+n)
    gb::Word val = _mem[0xFF00 + 0xBF];
    _loadAndExecute(0xF0, 0xBF);
    EXPECT_EQ(val, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xF1Test)
{
    // POP AF
    _cpu.registers().SP = 0x0038;
    _mem[0x0039] = 0xAB;
    _mem[0x0038] = 0xFE;
    _loadAndExecute(0xF1);
    EXPECT_EQ(0xAB, _cpu.registers().A);
    EXPECT_EQ(0xFE, _cpu.registers().F);
    EXPECT_EQ(_cpu.registers().SP, 0x003A);
    EXPECT_FLAGS(1,1,1,1);
}

TEST_F(CpuTest, Opcode0xF2Test)
{
    // LD A,(FF00+C)
    gb::Word val = _mem[0xFF00 + _cpu.registers().C];
    _loadAndExecute(0xF2);
    EXPECT_EQ(val, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xF3Test)
{
    // DI
    _cpu.setInterruptsEnabled(true);
    _loadAndExecute(0xF3);
    EXPECT_FALSE(_cpu.interruptsEnabled());
}

TEST_F(CpuTest, Opcode0xF5Test)
{
    // PUSH AF
    _cpu.registers().SP = 0x003A;
    _cpu.registers().AF = 0xABFE;
    _loadAndExecute(0xF5);
    EXPECT_EQ(_cpu.registers().A, _mem[0x0039]);
    EXPECT_EQ(_cpu.registers().F, _mem[0x0038]);
    EXPECT_EQ(_cpu.registers().SP, 0x0038);
    EXPECT_FLAGS(1,1,1,1);
}

TEST_F(CpuTest, Opcode0xF6Test)
{
    // OR n
    _cpu.registers().A = 0x0C;
    _loadAndExecute(0xF6, 0x18);
    EXPECT_EQ(0x1C, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().A = 0x00;
    _loadAndExecute(0xF6, 0x00);
    EXPECT_EQ(0x00, _cpu.registers().A);
    EXPECT_FLAGS(1,0,0,0);
}

TEST_F(CpuTest, Opcode0xF7Test)
{
    // RST 30H
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xF7);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x0030, _cpu.registers().PC);
    EXPECT_EQ(0x11, (int)_mem[0xFEFF]);
    EXPECT_EQ(0xAB, (int)_mem[0xFEFE]);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xF8Test)
{
    // LD HL,SP+dd
    _cpu.registers().SP = 0x0A05;
    
    gb::Dword val = _cpu.registers().SP + 38;
    _loadAndExecute(0xF8, 38);
    EXPECT_EQ(val, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);

    val = _cpu.registers().SP - 100;
    _loadAndExecute(0xF8, gb::toSigned8(-100));
    EXPECT_EQ(val, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);

    _cpu.registers().SP = 0x0FF0;

    val = _cpu.registers().SP + gb::toSigned8(0x0010);
    _loadAndExecute(0xF8, gb::toSigned8(0x0010));
    EXPECT_EQ(val, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,1,0);

    // Not a half carry -- only applies to highest byte
    _cpu.registers().SP = 0x000F;

    val = _cpu.registers().SP + gb::toSigned8(1);
    _loadAndExecute(0xF8, gb::toSigned8(1));
    EXPECT_EQ(val, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,0,0);

    // Full carry
    _cpu.registers().SP = 0xFFFF;

    val = _cpu.registers().SP + gb::toSigned8(10);
    _loadAndExecute(0xF8, gb::toSigned8(10));
    EXPECT_EQ(val, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,1,1);

    // Underflow carry
    _cpu.registers().SP = 0x000A;

    val = _cpu.registers().SP - 12;
    _loadAndExecute(0xF8, gb::toSigned8(-12));
    EXPECT_EQ(val, _cpu.registers().HL);
    EXPECT_FLAGS(0,0,1,1);
}

TEST_F(CpuTest, Opcode0xF9Test)
{
    // LD SP,HL
    gb::Dword val = _cpu.registers().HL;
    _loadAndExecute(0xF9);
    EXPECT_EQ(val, _cpu.registers().SP);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xFATest)
{
    // LD A,(nn)
    gb::Word val = _mem[0xB0F1];
    _loadAndExecute(0xFA, 0xF1, 0xB0);
    EXPECT_EQ(val, _cpu.registers().A);
    EXPECT_FLAGS(0,0,0,0);
}

TEST_F(CpuTest, Opcode0xFBTest)
{
    // EI
    EXPECT_FALSE(_cpu.interruptsEnabled());
    _loadAndExecute(0xFB);
    EXPECT_TRUE(_cpu.interruptsEnabled());
}

TEST_F(CpuTest, Opcode0xFETest)
{
    // CP n
    _cpu.registers().A = 0x0F;
    _loadAndExecute(0xFE, 0x02);
    EXPECT_FLAGS(0,1,0,0);

    _cpu.registers().A = 0x10;
    _loadAndExecute(0xFE, 0x01);
    EXPECT_FLAGS(0,1,1,0);

    _cpu.registers().A = 0x00;
    _loadAndExecute(0xFE, 0x01);
    EXPECT_FLAGS(0,1,1,1);

    _cpu.registers().A = 0xF1;
    _loadAndExecute(0xFE, 0xF1);
    EXPECT_FLAGS(1,1,0,0);
}

TEST_F(CpuTest, Opcode0xFFTest)
{
    // RST 38H
    _cpu.registers().SP = 0xFF00;
    _cpu.registers().PC = 0x11AA;
    _loadAndExecute(0xFF);
    EXPECT_EQ(0xFEFE, _cpu.registers().SP);
    EXPECT_EQ(0x0038, _cpu.registers().PC);
    EXPECT_EQ(0x11, (int)_mem[0xFEFF]);
    EXPECT_EQ(0xAB, (int)_mem[0xFEFE]);
    EXPECT_FLAGS(0,0,0,0);
}

