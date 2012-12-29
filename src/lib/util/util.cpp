#include "util.h"

std::string gb::toStr(gb::Byte val)
{
    char buf[5];
    sprintf(buf, "0x%02X", val);
    return std::string(buf);
}

std::string gb::toStr(gb::Word val)
{
    char buf[7];
    sprintf(buf, "0x%04X", val);
    return std::string(buf);
}

gb::Byte gb::toSigned8(int i)
{
    assert(i >= -128 && i < 128);
    gb::Byte w = abs(i);
    if (i < 0) {
        w = ~w + 1;
    }
    return w;
}

gb::Word gb::toSigned16(int i)
{
    assert(i >= -32768 && i < 32768);
    gb::Word w = abs(i);
    if (i < 0) {
        w = ~w + 1;
    }
    return w;
}

int gb::toInt8(gb::Byte val)
{
    return (-128)*((val & 0x80) >> 7) + (0x7F & val);
}

int gb::toInt16(gb::Word val)
{
    return (-32768)*((val & 0x8000) >> 15) + (0x7FFF & val);
}

