#ifndef GB_UTIL_H
#define GB_UTIL_H

#include <cassert>
#include <cstdlib>
#include <sstream>

#include "units.h"

namespace gb {

std::string toStr(gb::Byte n)
{
    char buf[5];
    sprintf(buf, "0x%02X", n);
    return buf;
}

gb::Byte toHex(const std::string& s)
{
    std::stringstream ss;
    ss << std::hex << s;
    
    int n;
    ss >> n;

    assert(n >= std::numeric_limits<gb::Byte>::min() && n <= std::numeric_limits<gb::Byte>::max());
    return static_cast<gb::Byte>(n);
}

}

#endif

