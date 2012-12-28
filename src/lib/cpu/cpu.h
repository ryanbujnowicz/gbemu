#ifndef GB_Cpu_H
#define GB_Cpu_H

#include <vector>

#include "cpu/addressable.h"
#include "util/units.h"
#include "util/util.h"

namespace gb {

/**
 * Emulates the modified GameBoy Z80 processor opcodes.
 */
class Cpu
{
public:

    struct Registers
    {
        union
        {
            struct
            {
                Word F;
                Word A;
            };    
            Dword AF;
        };

        union
        {
            struct
            {
                Word C;
                Word B;
            };    
            Dword BC;
        };

        union
        {
            struct
            {
                Word E;
                Word D;
            };    
            Dword DE;
        };

        union
        {
            struct
            {
                Word L;
                Word H;
            };    
            Dword HL;
        };

        Dword SP;
        Dword PC;
    };


    /**
     * References particular bits of the F (flag) register.
     */
    enum Flag
    {
        FlagZ = 7,
        FlagN = 6,
        FlagH = 5,
        FlagC = 4,
    };

    /**
     * Targets which instructions will set or retrieve values from.
     */
    enum Target
    {
        // 8bit
        RegB,
        RegC,
        RegD,
        RegE,
        RegH,
        RegL,
        MemHL,
        RegA,

        // 16bit
        RegBC,
        RegDE,
        RegHL,
        RegSP,

        // Memory access
        MemBC,
        MemDE,
        MemSP,
    };

    enum TargetType
    {
        TargetType8,
        TargetType16,
    };

public:
    Cpu()
    {
        reset();
    }
    ~Cpu() { }

    /*
     * Caller retains ownership of mem.
     */
    void setMemory(Addressable* mem) { _memory = mem; }

    Registers& registers()     { return _registers; }
    Word flag(Flag flag) const { return (_registers.F & (1<<flag)) >> flag; }

    void reset();
    void processNextInstruction();

    bool interruptsEnabled() const          { return _interruptsEnabled; }
    void setInterruptsEnabled(bool enabled) { _interruptsEnabled = enabled; }

    bool isStopped() const { return _isStopped; }
    bool isHalted() const { return _isHalted; }

protected:
    gb::Word _getArg8();
    gb::Dword _getArg16();

    Cpu::Target _getTarget8(gb::Word opcode) const;
    Cpu::Target _getTarget16(gb::Word opcode) const;
    Cpu::Target _convertToMemTarget(Cpu::Target target) const;
    Cpu::TargetType _getTargetType(Cpu::Target target) const;
    gb::Word _getTargetValue8(Cpu::Target target) const;
    gb::Dword _getTargetValue16(Cpu::Target target) const;

    gb::Word* _getTargetPtr8(Cpu::Target target); 
    gb::Dword* _getTargetPtr16(Cpu::Target target); 

    void _load(Cpu::Target target, gb::Word val);
    void _load(Cpu::Target target, gb::Dword val);
    void _load(Cpu::Target target, Cpu::Target source);
    void _loadToMem(gb::Dword addr, Cpu::Target source);
    void _loadFromMem(Cpu::Target target, gb::Dword addr);

    void _add8(Cpu::Target target, gb::Word val);
    void _add16(Cpu::Target target, gb::Dword val);
    void _sub8(Cpu::Target target, gb::Word val);
    void _sub16(Cpu::Target target, gb::Dword val);
    void _and(Cpu::Target target, gb::Word val);
    void _or(Cpu::Target target, gb::Word val);
    void _xor(Cpu::Target target, gb::Word val);
    void _complement(Cpu::Target target);

    void _bit(int bit, gb::Word val);
    void _set(int bit, Cpu::Target target);
    void _clear(int bit, Cpu::Target target);

    void _rlc(Cpu::Target target);
    void _rrc(Cpu::Target target);
    void _rl(Cpu::Target target);
    void _rr(Cpu::Target target);
    void _sla(Cpu::Target target);
    void _sra(Cpu::Target target);
    void _srl(Cpu::Target target);

    void _swap(Cpu::Target target);
    
    void _compare(gb::Word a, gb::Word b);

    void _call(gb::Dword addr);
    void _return();
    void _reset();

    void _assignFlag(Cpu::Flag flag, gb::Word val);
    void _assignFlags(int z, int n, int h, int c);
    
private:
    Registers _registers;
    Addressable* _memory;
    bool _interruptsEnabled;
    bool _isHalted;
    bool _isStopped;
};

}

#endif

