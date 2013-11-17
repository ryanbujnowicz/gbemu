#include "cpu.h"
using gb::Cpu;

#include "util.h"

#include <cassert>

void Cpu::reset()
{
    _registers.AF = 0x0000;
    _registers.BC = 0x0000;
    _registers.DE = 0x0000;
    _registers.HL = 0x0000;
    _registers.SP = 0x0000;
    _registers.PC = 0x0000;

    // Are interrupts enabled by default?
    _interruptsEnabled = true;
    _isHalted = false;
}

void Cpu::processNextInstruction()
{
    gb::Byte opcode = _getArg8();

    switch (opcode) {

        // NOP
        case 0x00:
            break;

            // LD rr,nn
        case 0x01: // BC
        case 0x11: // DE
        case 0x21: // HL
        case 0x31: // SP
            {
                _load(_getTarget16(opcode), _getArg16());
                break;
            }

            // LD (rr),A
        case 0x02: // BC
        case 0x12: // DE
            {
                _load(_convertToMemTarget(_getTarget16(opcode)), gb::Cpu::RegA);
                break;
            }

            // INC rr
        case 0x03: // BC
        case 0x13: // DE
        case 0x23: // HL
        case 0x33: // SP
            {
                // No flags are set, so just use a load
                Target target = _getTarget16(opcode);
                _load(target, static_cast<gb::Word>(_getTargetValue16(target) + 1));
                break;
            }

            // INC r
        case 0x04: // B
        case 0x0C: // C
        case 0x14: // D
        case 0x1C: // E
        case 0x24: // H
        case 0x2C: // L
        case 0x34: // HL
        case 0x3C: // A
            {
                // Carry bit is set by add, but not by inc
            gb::Byte cFlag = flag(FlagC);
            _add8(_getTarget8(opcode - 0x04), 1);
            _assignFlag(FlagC, cFlag);
            break;
        }

        // DEC r
        case 0x05: // B
        case 0x0D: // C
        case 0x15: // D
        case 0x1D: // E
        case 0x25: // H
        case 0x2D: // L
        case 0x35: // (HL)
        case 0x3D: // A
        {
            gb::Byte cFlag = flag(FlagC);
            _sub8(_getTarget8(opcode - 0x05), 1);
            _assignFlag(FlagC, cFlag);
            break;
        }

        // LD r,n
        case 0x06: // B    00000110  6
        case 0x0E: // C    00001110 14
        case 0x16: // D    00010110 22
        case 0x1E: // E    00001110 30
        case 0x26: // H    00100110 38
        case 0x2E: // L    00101110 46
        case 0x36: // (HL) 00110110 54
        case 0x3E: // A    00111110 62
        {
            _load(_getTarget8(opcode - 0x06), _getArg8());
            break;
        }

        // RLCA
        case 0x07:
        {
            _rlc(gb::Cpu::RegA);
            break;
        }

        // LD (nn),SP
        case 0x08:
        {
            _loadToMem(_getArg16(), gb::Cpu::RegSP);
            break;
        }

        // ADD HL,BC
        case 0x09:
        {
            // z flag is not affected
            gb::Byte zFlag = flag(FlagZ);
            _add16(gb::Cpu::RegHL, _getTargetValue16(gb::Cpu::RegBC));
            _assignFlag(gb::Cpu::FlagZ, zFlag);    
            break;
        }

        // LD A,(rr)
        case 0x0A: // BC
        case 0x1A: // DE
        {
            _load(gb::Cpu::RegA, _convertToMemTarget(_getTarget16(opcode)));
            break;
        }

        // DEC rr
        case 0x0B: // BC
        case 0x1B: // DE
        case 0x2B: // HL
        case 0x3B: // SP
        {
            // No flags are set, so just use a load
            Target target = _getTarget16(opcode);
            _load(target, static_cast<gb::Word>(_getTargetValue16(target) - 1));
            break;
        }

        // RRCA
        case 0x0F:
        {
            _rrc(gb::Cpu::RegA);
            break;
        }

        // STOP
        case 0x10:
        {
            _isStopped = true;
            break;
        }

        // RLA
        case 0x17:
        {
            _rl(gb::Cpu::RegA);
            break;
        }

        // JR (PC+e)
        case 0x18:
        {
            int offset = gb::toInt8(_getArg8());
            _registers.PC += offset;
            break;
        }

        // ADD HL,DE
        case 0x19:
        {
            // z flag is not affected
            gb::Byte zFlag = flag(FlagZ);
            _add16(gb::Cpu::RegHL, _getTargetValue16(gb::Cpu::RegDE));
            _assignFlag(gb::Cpu::FlagZ, zFlag);    
            break;
        }

        // RRA
        case 0x1F:
        {
            _rr(gb::Cpu::RegA);
            break;
        }

        // JR NZ,(PC+e)
        case 0x20:
        {
            int offset = gb::toInt8(_getArg8());
            if (!flag(gb::Cpu::FlagZ)) {
                _registers.PC += offset;
            }
            break;
        }

        // LDI (HL),A
        case 0x22:
        {
            (*_memory)[_registers.HL++] = _registers.A;
            break;
        }

        // DAA
        case 0x27:
        {
            int n = flag(gb::Cpu::FlagN);
            int h = flag(gb::Cpu::FlagH);
            int c = flag(gb::Cpu::FlagC);

            gb::Byte high = (_registers.A & 0xF0) >> 4;
            gb::Byte low = (_registers.A & 0x0F);

            int newC = 0;
            gb::Byte toAdd = 0;

            if (n == 0) {
                if (c == 0 && h == 0 && high >= 0 && high <= 9 && low >= 0 && low <= 9) {
                    newC = 0;
                    toAdd = 0x00;
                } else if (c == 0 && ((h == 0 && high >= 0x0 && high <= 0x8 && low >= 0xA && low <= 0xF) ||
                                      (h == 1 && high >= 0x0 && high <= 0x9 && low >= 0x0 && low <= 0x3))) {
                    newC = 0;
                    toAdd = 0x06;
                } else if (h == 0 && ((c == 0 && high >= 0xA && high <= 0xF && low >= 0x0 && low <= 0x9) ||
                                      (c == 1 && high >= 0x0 && high <= 0x2 && low >= 0x0 && low <= 0x9))) {
                    newC = 1;
                    toAdd = 0x60;
                } else if ((c == 0 && ((h == 0 && high >= 0x9 && high <= 0xF && low >= 0xA && low <= 0xF) ||
                                       (h == 1 && high >= 0xA && high <= 0xF && low >= 0x0 && low <= 0x3))) ||
                           (c == 1 && ((h == 0 && high >= 0x0 && high <= 0x2 && low >= 0xA && low <= 0xF) ||
                                       (h == 1 && high >= 0x0 && high <= 0x3 && low >= 0x0 && low <= 0x3)))) {
                    newC = 1;
                    toAdd = 0x66;
                }
            } else {
                if (c == 0 && h == 0 && high >= 0x0 && high <= 0x9 && low >= 0x0 && low <= 0x9) {
                    newC = 0;
                    toAdd = 0x00;
                } else if (c == 0 && h == 1 && high >= 0x0 && high <= 0x8 && low >= 0x6 && low <= 0xF) {
                    newC = 0;
                    toAdd = 0xFA;
                } else if (c == 1 && h == 0 && high >= 0x7 && high <= 0xF && low >= 0x0 && low <= 0x9) {
                    newC = 1;
                    toAdd = 0xA0;
                } else if (c == 1 && h == 1 && high >= 0x6 && high <= 0xF && low >= 0x6 && low <= 0xF) {
                    newC = 1;
                    toAdd = 0x9A;
                }
            }

            _registers.A += toAdd;
            _assignFlags(_registers.A == 0x00, flag(gb::Cpu::FlagN), 0, newC);
            break;
        }

        // JR Z,(PC+e)
        case 0x28:
        {
            int offset = gb::toInt8(_getArg8());
            if (flag(gb::Cpu::FlagZ)) {
                _registers.PC += offset;
            }
            break;
        }

        // ADD HL,HL
        case 0x29:
        {
            // z flag is not affected
            gb::Byte zFlag = flag(FlagZ);
            _add16(gb::Cpu::RegHL, _getTargetValue16(gb::Cpu::RegHL));
            _assignFlag(gb::Cpu::FlagZ, zFlag);    
            break;
        }

        // LDI A,(HL)
        case 0x2A:
        {
            _registers.A = (*_memory)[_registers.HL++];
            break;
        }

        // CPL
        case 0x2F:
        {
            _complement(gb::Cpu::RegA);
            break;
        }

        // JR NC,(PC+e)
        case 0x30:
        {
            int offset = gb::toInt8(_getArg8());
            if (!flag(gb::Cpu::FlagC)) {
                _registers.PC += offset;
            }
            break;
        }

        // LDD (HL),A
        case 0x32:
        {
            (*_memory)[_registers.HL--] = _registers.A;
            break;
        }

        // SCF
        case 0x37:
        {
            _assignFlag(gb::Cpu::FlagC, 1);
            _assignFlag(gb::Cpu::FlagN, 0);
            _assignFlag(gb::Cpu::FlagH, 0);
            break;
        }

        // JR C,(PC+e)
        case 0x38:
        {
            int offset = gb::toInt8(_getArg8());
            if (flag(gb::Cpu::FlagC)) {
                _registers.PC += offset;
            }
            break;
        }

        // ADD HL,SP
        case 0x39:
        {
            // z flag is not affected
            gb::Byte zFlag = flag(FlagZ);
            _add16(gb::Cpu::RegHL, _getTargetValue16(gb::Cpu::RegSP));
            _assignFlag(gb::Cpu::FlagZ, zFlag);    
            break;
        }

        // LDD A,(HL)
        case 0x3A:
        {
            _registers.A = (*_memory)[_registers.HL--];
            break;
        }

        // CCF
        case 0x3F:
        {
            _registers.F ^= (1 << gb::Cpu::FlagC);
            _assignFlag(gb::Cpu::FlagN, 0);
            _assignFlag(gb::Cpu::FlagH, 0);
            break;
        }

        // LD B,r
        case 0x40: // B
        case 0x41: // C
        case 0x42: // D
        case 0x43: // E
        case 0x44: // H
        case 0x45: // L
        case 0x46: // (HL)
        case 0x47: // A
        {
            _load(gb::Cpu::RegB, static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x40)));
            break;
        }

        // LD C,r
        case 0x48: // B
        case 0x49: // C
        case 0x4A: // D
        case 0x4B: // E
        case 0x4C: // H
        case 0x4D: // L
        case 0x4E: // (HL)
        case 0x4F: // A
        {
            _load(gb::Cpu::RegC, static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x48)));
            break;
        }

        // LD D,r
        case 0x50: // B
        case 0x51: // C
        case 0x52: // D
        case 0x53: // E
        case 0x54: // H
        case 0x55: // L
        case 0x56: // (HL)
        case 0x57: // A
        {
            _load(gb::Cpu::RegD, static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x50)));
            break;
        }

        // LD E,r
        case 0x58: // B
        case 0x59: // C
        case 0x5A: // D
        case 0x5B: // E
        case 0x5C: // H
        case 0x5D: // L
        case 0x5E: // (HL)
        case 0x5F: // A
        {
            _load(gb::Cpu::RegE, static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x58)));
            break;
        }

        // LD H,r
        case 0x60: // B
        case 0x61: // C
        case 0x62: // D
        case 0x63: // E
        case 0x64: // H
        case 0x65: // L
        case 0x66: // (HL)
        case 0x67: // A
        {
            _load(gb::Cpu::RegH, static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x60)));
            break;
        }

        // LD L,r
        case 0x68: // B
        case 0x69: // C
        case 0x6A: // D
        case 0x6B: // E
        case 0x6C: // H
        case 0x6D: // L
        case 0x6E: // (HL)
        case 0x6F: // A
        {
            _load(gb::Cpu::RegL, static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x68)));
            break;
        }

        // LD (HL),r
        case 0x70: // B
        case 0x71: // C
        case 0x72: // D
        case 0x73: // E
        case 0x74: // H
        case 0x75: // L
        case 0x77: // A
        {
            _load(gb::Cpu::MemHL, static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x70)));
            break;
        }

        // LD A,r
        case 0x78: // B
        case 0x79: // C
        case 0x7A: // D
        case 0x7B: // E
        case 0x7C: // H
        case 0x7D: // L
        case 0x7E: // (HL)
        case 0x7F: // A
        {
            _load(gb::Cpu::RegA, static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x78)));
            break;
        }

        // HALT
        case 0x76:
        {
            _isHalted = true;

            // HALT skips the next instruction if interrupts disabled
            if (!_interruptsEnabled) {
                _registers.PC++;
            }
            break;
        }

        // ADD A,r
        case 0x80: // B
        case 0x81: // C
        case 0x82: // D
        case 0x83: // E
        case 0x84: // H
        case 0x85: // L
        case 0x86: // (HL)
        case 0x87: // A
        {
            _add8(gb::Cpu::RegA, _getTargetValue8(_getOffsetTarget8(opcode, 0x80)));
            break;
        }

        // ADC A,r
        case 0x88: // B
        case 0x89: // C
        case 0x8A: // D
        case 0x8B: // E
        case 0x8C: // H
        case 0x8D: // L
        case 0x8E: // (HL)
        case 0x8F: // A
        {
            _add8(gb::Cpu::RegA, _getTargetValue8(_getOffsetTarget8(opcode, 0x88)) + flag(gb::Cpu::FlagC));
            break;
        }

        // SUB r
        case 0x90: // B
        case 0x91: // C
        case 0x92: // D
        case 0x93: // E
        case 0x94: // H
        case 0x95: // L
        case 0x96: // (HL)
        case 0x97: // A
        {
            _sub8(gb::Cpu::RegA, _getTargetValue8(_getOffsetTarget8(opcode, 0x90)));
            break;
        }

        // SBC A,r
        case 0x98: // B
        case 0x99: // C
        case 0x9A: // D
        case 0x9B: // E
        case 0x9C: // H
        case 0x9D: // L
        case 0x9E: // (HL)
        case 0x9F: // A
        {
            _sub8(gb::Cpu::RegA, _getTargetValue8(_getOffsetTarget8(opcode, 0x98)) + flag(gb::Cpu::FlagC));
            break;
        }

        // AND r
        case 0xA0: // B
        case 0xA1: // C
        case 0xA2: // D
        case 0xA3: // E
        case 0xA4: // H
        case 0xA5: // L
        case 0xA6: // (HL)
        case 0xA7: // A
        {
            _and(gb::Cpu::RegA, _getTargetValue8(_getOffsetTarget8(opcode, 0xA0)));
            break;
        }

        // XOR r
        case 0xA8: // B
        case 0xA9: // C
        case 0xAA: // D
        case 0xAB: // E
        case 0xAC: // H
        case 0xAD: // L
        case 0xAE: // (HL)
        case 0xAF: // A
        {
            _xor(gb::Cpu::RegA, _getTargetValue8(_getOffsetTarget8(opcode, 0xA8)));
            break;
        }

        // OR r
        case 0xB0: // B
        case 0xB1: // C
        case 0xB2: // D
        case 0xB3: // E
        case 0xB4: // H
        case 0xB5: // L
        case 0xB6: // (HL)
        case 0xB7: // A
        {
            _or(gb::Cpu::RegA, _getTargetValue8(_getOffsetTarget8(opcode, 0xB0)));
            break;
        }

        // CP r
        case 0xB8: // B
        case 0xB9: // C
        case 0xBA: // D
        case 0xBB: // E
        case 0xBC: // H
        case 0xBD: // L
        case 0xBE: // (HL)
        case 0xBF: // A
        {
            _compare(_getTargetValue8(gb::Cpu::RegA),
                     _getTargetValue8(_getOffsetTarget8(opcode, 0xB8)));
            break;
        }

        // RET NZ
        case 0xC0:
        {
            if (!flag(gb::Cpu::FlagZ)) {
                _return();
            }
            break;
        }

        // POP rr
        case 0xC1: // BC
        case 0xD1: // DE
        case 0xE1: // HL
        case 0xF1: // AF
        {
            gb::Word* data = nullptr; 
            switch (opcode) {
                case 0xC1: data = &_registers.BC; break;
                case 0xD1: data = &_registers.DE; break;
                case 0xE1: data = &_registers.HL; break;
                case 0xF1: data = &_registers.AF; break;
                default: assert("Unhandled switch case"); break;
            }

            gb::Byte low = (*_memory)[_registers.SP++];
            gb::Byte high = (*_memory)[_registers.SP++];
            *data = (high << 8) | low;
            break;
        }

        // JP NZ,(nn)
        case 0xC2:
        {
            gb::Word nn = _getArg16();
            if (!flag(gb::Cpu::FlagZ)) {
                _registers.PC = nn;
            }
            break;
        }

        // JP (nn)
        case 0xC3:
        {
            gb::Word nn = _getArg16();
            _registers.PC = nn;
            break;
        }


        // CALL NZ,(nn)
        case 0xC4:
        {
            gb::Word nn = _getArg16();
            if (!flag(gb::Cpu::FlagZ)) {
                _call(nn);
            }
            break;
        }

        // PUSH rr
        case 0xC5: // BC
        case 0xD5: // DE
        case 0xE5: // HL
        case 0xF5: // AF
        {
            gb::Word* data = nullptr; 
            switch (opcode) {
                case 0xC5: data = &_registers.BC; break;
                case 0xD5: data = &_registers.DE; break;
                case 0xE5: data = &_registers.HL; break;
                case 0xF5: data = &_registers.AF; break;
                default: assert("Unhandled switch case"); break;
            }

            (*_memory)[--_registers.SP] = ((*data) >> 8);
            (*_memory)[--_registers.SP] = ((*data) & 0x00FF);
            break;
        }

        // ADD A,n
        case 0xC6:
        {
            _add8(gb::Cpu::RegA, _getArg8());
            break;
        }

        // RST 0H
        case 0xC7:
        {
            _call(0x0000);
            break;
        }

        // RET Z
        case 0xC8:
        {
            if (flag(gb::Cpu::FlagZ)) {
                _return();
            }
            break;
        }

        // RET
        case 0xC9:
        {
            _return();
            break;
        }

        // JP Z,(nn)
        case 0xCA:
        {
            gb::Word nn = _getArg16();
            if (flag(gb::Cpu::FlagZ)) {
                _registers.PC = nn;
            }
            break;
        }

        // CB sub-ops
        case 0xCB:
        {
            opcode = _getArg8();
            switch (opcode) {

                // RLC r
                case 0x00: // B
                case 0x01: // C
                case 0x02: // D
                case 0x03: // E
                case 0x04: // H
                case 0x05: // L
                case 0x06: // (HL)
                case 0x07: // A
                {
                    _rlc(static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x00)));
                    break;
                }

                // RRC r
                case 0x08: // B
                case 0x09: // C
                case 0x0A: // D
                case 0x0B: // E
                case 0x0C: // H
                case 0x0D: // L
                case 0x0E: // (HL)
                case 0x0F: // A
                {
                    _rrc(static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x08)));
                    break;
                }

                // RL r
                case 0x10: // B
                case 0x11: // C
                case 0x12: // D
                case 0x13: // E
                case 0x14: // H
                case 0x15: // L
                case 0x16: // (HL)
                case 0x17: // A
                {
                    _rl(static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x10)));
                    break;
                }

                // RR r
                case 0x18: // B
                case 0x19: // C
                case 0x1A: // D
                case 0x1B: // E
                case 0x1C: // H
                case 0x1D: // L
                case 0x1E: // (HL)
                case 0x1F: // A
                {
                    _rr(static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x18)));
                    break;
                }

                // SLA r
                case 0x20: // B
                case 0x21: // C
                case 0x22: // D
                case 0x23: // E
                case 0x24: // H
                case 0x25: // L
                case 0x26: // (HL)
                case 0x27: // A
                {
                    _sla(static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x20)));
                    break;
                }

                // SRA r
                case 0x28: // B
                case 0x29: // C
                case 0x2A: // D
                case 0x2B: // E
                case 0x2C: // H
                case 0x2D: // L
                case 0x2E: // (HL)
                case 0x2F: // A
                {
                    _sra(static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x28)));
                    break;
                }

                // SWAP r
                case 0x30: // B
                case 0x31: // C
                case 0x32: // D
                case 0x33: // E
                case 0x34: // H
                case 0x35: // L
                case 0x36: // (HL)
                case 0x37: // A
                {
                    _swap(static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x30)));
                    break;
                }

                // SRL r
                case 0x38: // B
                case 0x39: // C
                case 0x3A: // D
                case 0x3B: // E
                case 0x3C: // H
                case 0x3D: // L
                case 0x3E: // (HL)
                case 0x3F: // A
                {
                    _srl(static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - 0x38)));
                    break;
                }

                // BIT 0,r
                case 0x40: // B
                case 0x41: // C
                case 0x42: // D
                case 0x43: // E
                case 0x44: // H
                case 0x45: // L
                case 0x46: // (HL)
                case 0x47: // A
                {
                    _bit(0, _getTargetValue8(_getOffsetTarget8(opcode, 0x40)));
                    break;
                }

                // BIT 1,r
                case 0x48: // B
                case 0x49: // C
                case 0x4A: // D
                case 0x4B: // E
                case 0x4C: // H
                case 0x4D: // L
                case 0x4E: // (HL)
                case 0x4F: // A
                {
                    _bit(1, _getTargetValue8(_getOffsetTarget8(opcode, 0x48)));
                    break;
                }

                // BIT 2,r
                case 0x50: // B
                case 0x51: // C
                case 0x52: // D
                case 0x53: // E
                case 0x54: // H
                case 0x55: // L
                case 0x56: // (HL)
                case 0x57: // A
                {
                    _bit(2, _getTargetValue8(_getOffsetTarget8(opcode, 0x50)));
                    break;
                }

                // BIT 3,r
                case 0x58: // B
                case 0x59: // C
                case 0x5A: // D
                case 0x5B: // E
                case 0x5C: // H
                case 0x5D: // L
                case 0x5E: // (HL)
                case 0x5F: // A
                {
                    _bit(3, _getTargetValue8(_getOffsetTarget8(opcode, 0x58)));
                    break;
                }

                // BIT 4,r
                case 0x60: // B
                case 0x61: // C
                case 0x62: // D
                case 0x63: // E
                case 0x64: // H
                case 0x65: // L
                case 0x66: // (HL)
                case 0x67: // A
                {
                    _bit(4, _getTargetValue8(_getOffsetTarget8(opcode, 0x60)));
                    break;
                }

                // BIT 5,r
                case 0x68: // B
                case 0x69: // C
                case 0x6A: // D
                case 0x6B: // E
                case 0x6C: // H
                case 0x6D: // L
                case 0x6E: // (HL)
                case 0x6F: // A
                {
                    _bit(5, _getTargetValue8(_getOffsetTarget8(opcode, 0x68)));
                    break;
                }

                // BIT 6,r
                case 0x70: // B
                case 0x71: // C
                case 0x72: // D
                case 0x73: // E
                case 0x74: // H
                case 0x75: // L
                case 0x76: // (HL)
                case 0x77: // A
                {
                    _bit(6, _getTargetValue8(_getOffsetTarget8(opcode, 0x70)));
                    break;
                }

                // BIT 7,r
                case 0x78: // B
                case 0x79: // C
                case 0x7A: // D
                case 0x7B: // E
                case 0x7C: // H
                case 0x7D: // L
                case 0x7E: // (HL)
                case 0x7F: // A
                {
                    _bit(7, _getTargetValue8(_getOffsetTarget8(opcode, 0x78)));
                    break;
                }

                // RES 0,r
                case 0x80: // B
                case 0x81: // C
                case 0x82: // D
                case 0x83: // E
                case 0x84: // H
                case 0x85: // L
                case 0x86: // (HL)
                case 0x87: // A
                {
                    _clear(0, _getOffsetTarget8(opcode, 0x80));
                    break;
                }

                // RES 1,r
                case 0x88: // B
                case 0x89: // C
                case 0x8A: // D
                case 0x8B: // E
                case 0x8C: // H
                case 0x8D: // L
                case 0x8E: // (HL)
                case 0x8F: // A
                {
                    _clear(1, _getOffsetTarget8(opcode, 0x88));
                    break;
                }

                // RES 2,r
                case 0x90: // B
                case 0x91: // C
                case 0x92: // D
                case 0x93: // E
                case 0x94: // H
                case 0x95: // L
                case 0x96: // (HL)
                case 0x97: // A
                {
                    _clear(2, _getOffsetTarget8(opcode, 0x90));
                    break;
                }

                // RES 3,r
                case 0x98: // B
                case 0x99: // C
                case 0x9A: // D
                case 0x9B: // E
                case 0x9C: // H
                case 0x9D: // L
                case 0x9E: // (HL)
                case 0x9F: // A
                {
                    _clear(3, _getOffsetTarget8(opcode, 0x98));
                    break;
                }

                // RES 4,r
                case 0xA0: // B
                case 0xA1: // C
                case 0xA2: // D
                case 0xA3: // E
                case 0xA4: // H
                case 0xA5: // L
                case 0xA6: // (HL)
                case 0xA7: // A
                {
                    _clear(4, _getOffsetTarget8(opcode, 0xA0));
                    break;
                }

                // RES 5,r
                case 0xA8: // B
                case 0xA9: // C
                case 0xAA: // D
                case 0xAB: // E
                case 0xAC: // H
                case 0xAD: // L
                case 0xAE: // (HL)
                case 0xAF: // A
                {
                    _clear(5, _getOffsetTarget8(opcode, 0xA8));
                    break;
                }

                // RES 6,r
                case 0xB0: // B
                case 0xB1: // C
                case 0xB2: // D
                case 0xB3: // E
                case 0xB4: // H
                case 0xB5: // L
                case 0xB6: // (HL)
                case 0xB7: // A
                {
                    _clear(6, _getOffsetTarget8(opcode, 0xB0));
                    break;
                }

                // RES 7,r
                case 0xB8: // B
                case 0xB9: // C
                case 0xBA: // D
                case 0xBB: // E
                case 0xBC: // H
                case 0xBD: // L
                case 0xBE: // (HL)
                case 0xBF: // A
                {
                    _clear(7, _getOffsetTarget8(opcode, 0xB8));
                    break;
                }

                // SET 0,r
                case 0xC0: // B
                case 0xC1: // C
                case 0xC2: // D
                case 0xC3: // E
                case 0xC4: // H
                case 0xC5: // L
                case 0xC6: // (HL)
                case 0xC7: // A
                {
                    _set(0, _getOffsetTarget8(opcode, 0xC0));
                    break;
                }

                // SET 1,r
                case 0xC8: // B
                case 0xC9: // C
                case 0xCA: // D
                case 0xCB: // E
                case 0xCC: // H
                case 0xCD: // L
                case 0xCE: // (HL)
                case 0xCF: // A
                {
                    _set(1, _getOffsetTarget8(opcode, 0xC8));
                    break;
                }

                // SET 2,r
                case 0xD0: // B
                case 0xD1: // C
                case 0xD2: // D
                case 0xD3: // E
                case 0xD4: // H
                case 0xD5: // L
                case 0xD6: // (HL)
                case 0xD7: // A
                {
                    _set(2, _getOffsetTarget8(opcode, 0xD0));
                    break;
                }

                // SET 3,r
                case 0xD8: // B
                case 0xD9: // C
                case 0xDA: // D
                case 0xDB: // E
                case 0xDC: // H
                case 0xDD: // L
                case 0xDE: // (HL)
                case 0xDF: // A
                {
                    _set(3, _getOffsetTarget8(opcode, 0xD8));
                    break;
                }

                // SET 4,r
                case 0xE0: // B
                case 0xE1: // C
                case 0xE2: // D
                case 0xE3: // E
                case 0xE4: // H
                case 0xE5: // L
                case 0xE6: // (HL)
                case 0xE7: // A
                {
                    _set(4, _getOffsetTarget8(opcode, 0xE0));
                    break;
                }

                // SET 5,r
                case 0xE8: // B
                case 0xE9: // C
                case 0xEA: // D
                case 0xEB: // E
                case 0xEC: // H
                case 0xED: // L
                case 0xEE: // (HL)
                case 0xEF: // A
                {
                    _set(5, _getOffsetTarget8(opcode, 0xE8));
                    break;
                }

                // SET 6,r
                case 0xF0: // B
                case 0xF1: // C
                case 0xF2: // D
                case 0xF3: // E
                case 0xF4: // H
                case 0xF5: // L
                case 0xF6: // (HL)
                case 0xF7: // A
                {
                    _set(6, _getOffsetTarget8(opcode, 0xF0));
                    break;
                }

                // SET 7,r
                case 0xF8: // B
                case 0xF9: // C
                case 0xFA: // D
                case 0xFB: // E
                case 0xFC: // H
                case 0xFD: // L
                case 0xFE: // (HL)
                case 0xFF: // A
                {
                    _set(7, _getOffsetTarget8(opcode, 0xF8));
                    break;
                }
            }
        } break;

        // CALL Z,(nn)
        case 0xCC:
        {
            gb::Word nn = _getArg16();
            if (flag(gb::Cpu::FlagZ)) {
                _call(nn);
            }
            break;
        }

        // CALL (nn)
        case 0xCD:
        {
            gb::Word nn = _getArg16();
            _call(nn);
            break;
        }

        // ADC A,n
        case 0xCE:
        {
            gb::Byte n = _getArg8();
            _add8(gb::Cpu::RegA, n + flag(gb::Cpu::FlagC));
            break;
        }

        // RST 8H
        case 0xCF:
        {
            _call(0x0008);
            break;
        }

        // RET NC
        case 0xD0:
        {
            if (!flag(gb::Cpu::FlagC)) {
                _return();
            }
            break;
        }

        // JP NC,(nn)
        case 0xD2:
        {
            gb::Word nn = _getArg16();
            if (!flag(gb::Cpu::FlagC)) {
                _registers.PC = nn;
            }
            break;
        }

        // CALL NC,(nn)
        case 0xD4:
        {
            gb::Word nn = _getArg16();
            if (!flag(gb::Cpu::FlagC)) {
                _call(nn);
            }
            break;
        }

        // SUB n
        case 0xD6:
        {
            _sub8(gb::Cpu::RegA, _getArg8());
            break;
        }

        // RST 10H
        case 0xD7:
        {
            _call(0x0010);
            break;
        }

        // RET C
        case 0xD8:
        {
            if (flag(gb::Cpu::FlagC)) {
                _return();
            }
            break;
        }

        // RETI
        case 0xD9:
        {
            _return();
            setInterruptsEnabled(true);
            break;
        }

        // JP C,(nn)
        case 0xDA:
        {
            gb::Word nn = _getArg16();
            if (flag(gb::Cpu::FlagC)) {
                _registers.PC = nn;
            }
            break;
        }

        // CALL C,(nn)
        case 0xDC:
        {
            gb::Word nn = _getArg16();
            if (flag(gb::Cpu::FlagC)) {
                _call(nn);
            }
            break;
        }

        // SBC A,n
        case 0xDE:
        {
            gb::Byte n = _getArg8();
            _sub8(gb::Cpu::RegA, n + flag(gb::Cpu::FlagC));
            break;
        }

        // RST 18H
        case 0xDF:
        {
            _call(0x0018);
            break;
        }

        // LD (FF00+n),A
        case 0xE0:
            _loadToMem(0xFF00 + _getArg8(), gb::Cpu::RegA);
            break;

        // LD (FF00+C),A
        case 0xE2:
            _loadToMem(0xFF00 + _getTargetValue8(gb::Cpu::RegC), gb::Cpu::RegA);
            break;

        // AND n
        case 0xE6:
        {
            _and(gb::Cpu::RegA, _getArg8());
            break;
        }

        // RST 20H
        case 0xE7:
        {
            _call(0x0020);
            break;
        }

        // ADD SP,dd
        case 0xE8:
        {
            gb::Byte offset = _getArg8();
            int ioffset = gb::toInt8(offset);
            if (ioffset >= 0) {
                _add16(gb::Cpu::RegSP, offset);
            } else {
                gb::Byte absOffset = abs(ioffset);
                _sub16(gb::Cpu::RegSP, absOffset);
            }
            _assignFlag(gb::Cpu::FlagZ, 0);
            _assignFlag(gb::Cpu::FlagN, 0);
            break;
        }

        // JP (HL)
        case 0xE9:
        {
            _registers.PC = (*_memory)[_registers.HL];
            break;
        }

        // LD (nn),A
        case 0xEA:
        {
            _loadToMem(_getArg16(), gb::Cpu::RegA);
            break;
        }

        // XOR n
        case 0xEE:
        {
            _xor(gb::Cpu::RegA, _getArg8());
            break;
        }

        // RST 28H
        case 0xEF:
        {
            _call(0x0028);
            break;
        }

        // LD A,(FF00+n)
        case 0xF0:
        {
            _loadFromMem(gb::Cpu::RegA, 0xFF00 + _getArg8());
            break;
        }

        // LD A,(FF00+C)
        case 0xF2:
        {
            _loadFromMem(gb::Cpu::RegA, 0xFF00 + _getTargetValue8(gb::Cpu::RegC));
            break;
        }

        // DI
        case 0xF3:
        {
            setInterruptsEnabled(false);
            break;
        }

        // OR n
        case 0xF6:
        {
            _or(gb::Cpu::RegA, _getArg8());
            break;
        }

        // RST 30H
        case 0xF7:
        {
            _call(0x0030);
            break;
        }

        // LD HL,SP+dd
        case 0xF8:
        {
            // TODO: this can be done better

            // This is a special one, we want to add16 but the result doesn't
            // go into the same register.
            gb::Word* data = _getTargetPtr16(gb::Cpu::RegHL);
            gb::Word sp = _getTargetValue16(gb::Cpu::RegSP);

            // This is an 8 bit signed, convert to a 16 bit signed
            gb::Byte arg = _getArg8();
            int offset = gb::toInt8(arg);
            gb::Word absOffset = abs(offset);

            gb::Word val = 0x00;

            // Do either a SUB or ADD check depending on sign of val
            if (offset > 0) {
                val = sp + absOffset;
                int fullRes = sp + absOffset;
                _assignFlags(0,
                             0, 
                             (((sp&0x0FFF) + (absOffset&0x0FFF))&0x1000) == 0x1000, 
                             fullRes > std::numeric_limits<gb::Word>::max());
            } else {
                val = sp - absOffset;
                int fullRes = sp - absOffset;
                _assignFlags(0,
                             0, 
                             (static_cast<int>(sp&0x0FFF) - static_cast<int>(absOffset&0x0FFF)) < 0,
                             fullRes < 0);
            }

            (*data) = val;
            break;
        }

        // LD SP,HL
        case 0xF9:
        {
            _load(gb::Cpu::RegSP, _getTargetValue16(gb::Cpu::RegHL));
            break;
        }

        // LD A,(nn)
        case 0xFA:
        {
            _loadFromMem(gb::Cpu::RegA, _getArg16());
            break;
        }

        // EI
        case 0xFB:
        {
            setInterruptsEnabled(true);
            break;
        }

        // CP n
        case 0xFE:
        {
            _compare(_getTargetValue8(gb::Cpu::RegA), _getArg8());
            break;
        }

        // RST 38H
        case 0xFF:
        {
            _call(0x0038);
            break;
        }

        default:
        {
            assert(false && "Unhandled opcode case");
            break;
        }
    }
}

gb::Byte Cpu::_getArg8()
{
    gb::Byte arg = (*_memory)[_registers.PC++];
    return arg;
}

gb::Word Cpu::_getArg16()
{
    gb::Word a = (*_memory)[_registers.PC++];
    gb::Word b = (*_memory)[_registers.PC++];
    return ((b << 8) | a);
}

Cpu::Target Cpu::_getTarget8(gb::Byte opcode) const
{
    return static_cast<Cpu::Target>(Cpu::RegB + (opcode/8));
}

Cpu::Target Cpu::_getTarget16(gb::Byte opcode) const
{
    return static_cast<Cpu::Target>(Cpu::RegBC + ((opcode & 0xF0) >> 4));
}

Cpu::Target Cpu::_getOffsetTarget8(gb::Byte opcode, gb::Byte offset) const
{
    return static_cast<gb::Cpu::Target>(gb::Cpu::RegB + (opcode - offset));
}

Cpu::Target Cpu::_convertToMemTarget(Cpu::Target target) const
{
    // No-op for existing mem targets and 8bit registers
    switch (target) {
        case Cpu::RegBC:    return Cpu::MemBC;
        case Cpu::RegDE:    return Cpu::MemDE;
        case Cpu::RegHL:    return Cpu::MemHL;
        case Cpu::RegSP:    return Cpu::MemSP;
        default:            assert(false && "Switch case not handled"); break;
    }        
}

Cpu::TargetType Cpu::_getTargetType(Cpu::Target target) const
{
    switch (target) {
        case Cpu::RegB: 
        case Cpu::RegC:
        case Cpu::RegD:
        case Cpu::RegE:
        case Cpu::RegH:
        case Cpu::RegL:
        case Cpu::MemHL:
        case Cpu::RegA:
        case Cpu::MemBC:
        case Cpu::MemDE:
        case Cpu::MemSP:
            return Cpu::TargetType8;

        case Cpu::RegBC:
        case Cpu::RegDE:
        case Cpu::RegHL:
        case Cpu::RegSP:
            return Cpu::TargetType16;

        default:
            assert(false && "Switch case not handled");
            break;
    }
}

gb::Byte Cpu::_getTargetValue8(Cpu::Target target) const
{
    switch (target) {
        case Cpu::RegB:     return _registers.B;
        case Cpu::RegC:     return _registers.C;
        case Cpu::RegD:     return _registers.D;
        case Cpu::RegE:     return _registers.E;
        case Cpu::RegH:     return _registers.H;
        case Cpu::RegL:     return _registers.L;
        case Cpu::MemHL:    return (*_memory)[_registers.HL];
        case Cpu::RegA:     return _registers.A;
        case Cpu::MemBC:    return (*_memory)[_registers.BC];
        case Cpu::MemDE:    return (*_memory)[_registers.DE];
        case Cpu::MemSP:    return (*_memory)[_registers.SP];
        default:            assert(false && "Switch case not handled"); break;
    }        
}

gb::Word Cpu::_getTargetValue16(Cpu::Target target) const
{
    switch (target) {
        case Cpu::RegBC:    return _registers.BC;
        case Cpu::RegDE:    return _registers.DE;
        case Cpu::RegHL:    return _registers.HL;
        case Cpu::RegSP:    return _registers.SP;
        default:            assert(false && "Switch case not handled"); break;
    }        
}

gb::Byte* Cpu::_getTargetPtr8(Cpu::Target target)
{
    switch (target) {
        case Cpu::RegB:     return &_registers.B;
        case Cpu::RegC:     return &_registers.C;
        case Cpu::RegD:     return &_registers.D;
        case Cpu::RegE:     return &_registers.E;
        case Cpu::RegH:     return &_registers.H;
        case Cpu::RegL:     return &_registers.L;
        case Cpu::MemHL:    return &(*_memory)[_registers.HL];
        case Cpu::RegA:     return &_registers.A;
        case Cpu::MemBC:    return &(*_memory)[_registers.BC];
        case Cpu::MemDE:    return &(*_memory)[_registers.DE];
        case Cpu::MemSP:    return &(*_memory)[_registers.SP];
        default:            assert(false && "Switch case not handled");
    }
    return nullptr;
}

gb::Word* Cpu::_getTargetPtr16(Cpu::Target target)
{
    switch (target) {
        case Cpu::RegBC:     return &_registers.BC;
        case Cpu::RegDE:     return &_registers.DE;
        case Cpu::RegHL:     return &_registers.HL;
        case Cpu::RegSP:     return &_registers.SP;
        default:            assert(false && "Switch case not handled");
    }
    return nullptr;
}

void Cpu::_load(Cpu::Target target, gb::Byte n)
{
    switch (target) {
        case Cpu::RegB:     _registers.B = n;  break;
        case Cpu::RegC:     _registers.C = n;  break;
        case Cpu::RegD:     _registers.D = n;  break;
        case Cpu::RegE:     _registers.E = n;  break;
        case Cpu::RegH:     _registers.H = n;  break;
        case Cpu::RegL:     _registers.L = n;  break;
        case Cpu::MemHL:    (*_memory)[_registers.HL] = n;  break;
        case Cpu::RegA:     _registers.A = n;  break;
        case Cpu::MemBC:    (*_memory)[_registers.BC] = n;  break;
        case Cpu::MemDE:    (*_memory)[_registers.DE] = n;  break;
        case Cpu::MemSP:    (*_memory)[_registers.SP] = n;  break;
        default:            assert(false && "Switch case not handled"); break;
    }        
}

void Cpu::_load(Cpu::Target target, gb::Word nn)
{
    switch (target) {
        case Cpu::RegBC:    _registers.BC = nn; break;
        case Cpu::RegDE:    _registers.DE = nn; break;
        case Cpu::RegHL:    _registers.HL = nn; break;
        case Cpu::RegSP:    _registers.SP = nn; break;
        default:            assert(false && "Switch case not handled"); break;
    }
}

void Cpu::_load(Cpu::Target target, Cpu::Target source)
{
    Cpu::TargetType sourceType = _getTargetType(source);
    if (sourceType == Cpu::TargetType8) {
        gb::Byte val = _getTargetValue8(source);
        _load(target, val);
    } else if (sourceType == Cpu::TargetType16) {
        gb::Word val = _getTargetValue16(source);
        _load(target, val);
    } else {
        assert(false && "Unhandled target type");
    }
}

void Cpu::_loadToMem(gb::Word addr, Cpu::Target source)
{
    Cpu::TargetType sourceType = _getTargetType(source);
    if (sourceType == Cpu::TargetType8) {
        (*_memory)[addr] = _getTargetValue8(source);
    } else if (sourceType == Cpu::TargetType16) {
        gb::Word val = _getTargetValue16(source);
        (*_memory)[addr] = val & 0x00FF; 
        (*_memory)[addr + 1] = val & 0xFF00; 
    } else {
        assert(false && "Unhandled target type");
    }

}

void Cpu::_loadFromMem(Cpu::Target target, gb::Word addr)
{
    _load(target, (*_memory)[addr]);
}

void Cpu::_add8(Cpu::Target target, gb::Byte val)
{
    gb::Byte* data = _getTargetPtr8(target);

    int fullRes = *data + val;
    gb::Byte res = static_cast<gb::Byte>(fullRes);
    _assignFlags(res == 0, 
                 0, 
                 ((((*data)&0x0F) + (val&0x0F))&0x10) == 0x10, 
                 fullRes > std::numeric_limits<gb::Byte>::max());
    *data = res;
}

void Cpu::_add16(Cpu::Target target, gb::Word val)
{
    gb::Word* data = _getTargetPtr16(target);

    int fullRes = *data + val;
    gb::Word res = static_cast<gb::Word>(fullRes);
    _assignFlags(res == 0, 
                 0, 
                 ((((*data)&0x0FFF) + (val&0x0FFF))&0x1000) == 0x1000, 
                 fullRes > std::numeric_limits<gb::Word>::max());
    *data = res;
}

void Cpu::_sub8(Cpu::Target target, gb::Byte val)
{
    gb::Byte* data = _getTargetPtr8(target);

    int fullRes = *data - val;
    gb::Byte res = static_cast<gb::Byte>(fullRes);
    _assignFlags(res == 0, 
                 1, 
                 (static_cast<int>((*data)&0x0F) - static_cast<int>(val&0x0F)) < 0,
                 fullRes < 0);
    *data = res;
}

void Cpu::_sub16(Cpu::Target target, gb::Word val)
{
    gb::Word* data = _getTargetPtr16(target);

    int fullRes = *data - val;
    gb::Word res = static_cast<gb::Word>(fullRes);
    _assignFlags(res == 0, 
                 1, 
                 (static_cast<int>((*data)&0x0FFF) - static_cast<int>(val&0x0FFF)) < 0,
                 fullRes < 0);
    *data = res;
}

void Cpu::_and(Cpu::Target target, gb::Byte val)
{
    gb::Byte* data = _getTargetPtr8(target);
    *data &= val;
    _assignFlags(*data == 0, 0, 1, 0); 
}

void Cpu::_or(Cpu::Target target, gb::Byte val)
{
    gb::Byte* data = _getTargetPtr8(target);
    *data |= val;
    _assignFlags(*data == 0, 0, 0, 0); 
}

void Cpu::_xor(Cpu::Target target, gb::Byte val)
{
    gb::Byte* data = _getTargetPtr8(target);
    *data ^= val;
    _assignFlags(*data == 0, 0, 0, 0); 
}

void Cpu::_complement(Cpu::Target target)
{
    gb::Byte* data = _getTargetPtr8(target);
    *data = ~(*data);
    _assignFlags(flag(FlagZ), 1, 1, flag(FlagC)); 
}

void Cpu::_bit(int bit, gb::Byte val)
{
    int zero = 0;
    if ((val & (1 << bit)) == 0x00) {
        zero = 1; 
    }

    _assignFlag(gb::Cpu::FlagZ, zero);
    _assignFlag(gb::Cpu::FlagN, 0);
    _assignFlag(gb::Cpu::FlagH, 1);
}

void Cpu::_set(int bit, gb::Cpu::Target target)
{
    gb::Byte* data = _getTargetPtr8(target);
    (*data) |= (1 << bit);
}

void Cpu::_clear(int bit, gb::Cpu::Target target)
{
    gb::Byte* data = _getTargetPtr8(target);
    (*data) &= ~(1 << bit);
}

void Cpu::_rlc(gb::Cpu::Target target)
{
    gb::Byte* data = _getTargetPtr8(target);
    gb::Byte carry = ((*data) >> 7);
    (*data) = ((*data) << 1) | carry;
    _assignFlags((*data) == 0, 0, 0, carry);
}

void Cpu::_rrc(gb::Cpu::Target target)
{
    gb::Byte* data = _getTargetPtr8(target);
    gb::Byte carry = ((*data) & 0x01);
    (*data) = ((*data) >> 1) | (carry << 7);
    _assignFlags((*data) == 0, 0, 0, carry);
}

void Cpu::_rl(gb::Cpu::Target target)
{
    gb::Byte* data = _getTargetPtr8(target);
    gb::Byte carry = ((*data) >> 7);
    (*data) = ((*data) << 1) | flag(gb::Cpu::FlagC);
    _assignFlags((*data) == 0, 0, 0, carry);
}

void Cpu::_rr(gb::Cpu::Target target)
{
    gb::Byte* data = _getTargetPtr8(target);
    gb::Byte carry = ((*data) & 0x01);
    (*data) = ((*data) >> 1) | (flag(gb::Cpu::FlagC) << 7);
    _assignFlags((*data) == 0, 0, 0, carry);
}

void Cpu::_sla(gb::Cpu::Target target)
{
    gb::Byte* data = _getTargetPtr8(target);
    gb::Byte carry = ((*data) & 0x80) >> 7;
    (*data) = ((*data) << 1);
    _assignFlags((*data) == 0, 0, 0, carry);
}

void Cpu::_sra(gb::Cpu::Target target)
{
    gb::Byte* data = _getTargetPtr8(target);
    gb::Byte carry = ((*data) & 0x01);
    (*data) = ((*data) >> 1) | ((*data) & 0x80);
    _assignFlags((*data) == 0, 0, 0, carry);
}

void Cpu::_srl(gb::Cpu::Target target)
{
    gb::Byte* data = _getTargetPtr8(target);
    gb::Byte carry = ((*data) & 0x01);
    (*data) >>= 1;
    _assignFlags((*data) == 0, 0, 0, carry);
}

void Cpu::_swap(gb::Cpu::Target target)
{
    gb::Byte* data = _getTargetPtr8(target);
    (*data) = ((*data) << 4) | (((*data) & 0xF0) >> 4);
    _assignFlags((*data) == 0, 0, 0, 0);
}

void Cpu::_compare(gb::Byte a, gb::Byte b)
{
    // Similar to a subtract but no register values change
    int fullRes = a - b;
    gb::Byte res = static_cast<gb::Byte>(fullRes);
    _assignFlags(res == 0, 
                 1, 
                 (static_cast<int>(a&0x0F) - static_cast<int>(b&0x0F)) < 0,
                 fullRes < 0);
}

void Cpu::_call(gb::Word addr)
{
    (*_memory)[--_registers.SP] = (_registers.PC >> 8);
    (*_memory)[--_registers.SP] = (_registers.PC & 0x00FF);
    _registers.PC = addr;
}

void Cpu::_return()
{
    gb::Byte low = (*_memory)[_registers.SP++];
    gb::Byte high = (*_memory)[_registers.SP++];
    gb::Word addr = (high << 8) | low;
    _registers.PC = addr;
}

void Cpu::_assignFlag(Cpu::Flag flag, gb::Byte val)
{
    val = val & 0x1;
    _registers.F &= ~(1 << flag);
    _registers.F |= (val << flag);
}

void Cpu::_assignFlags(int z, int n, int h, int c)
{
    _registers.F = (z << FlagZ) | (n << FlagN) | (h << FlagH) | (c << FlagC);
}

