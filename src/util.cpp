#include "util.h"

std::string gb::toStr(gb::Word val)
{
    char buf[5];
    sprintf(buf, "0x%02X", val);
    return std::string(buf);
}

std::string gb::toStr(gb::Dword val)
{
    char buf[7];
    sprintf(buf, "0x%04X", val);
    return std::string(buf);
}

