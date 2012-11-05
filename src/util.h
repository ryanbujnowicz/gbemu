#ifndef GB_UTIL_H
#define GB_UTIL_H

#include <cassert>
#include <cstdlib>
#include <sstream>

#include "units.h"

namespace gb {

std::string toStr(gb::Word n)
{
    char buf[5];
    sprintf(buf, "0x%02X", n);
    return buf;
}

std::string toStr(gb::Dword n)
{
    char buf[7];
    sprintf(buf, "0x%04X", n);
    return buf;
}

template <typename T>
T toHex(const std::string& s)
{
    std::stringstream ss;
    ss << std::hex << s;
    
    size_t n;
    ss >> n;

    assert(n >= std::numeric_limits<T>::min() && n <= std::numeric_limits<T>::max());
    return static_cast<T>(n);
}

}

#endif

