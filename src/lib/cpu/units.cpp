#include "units.h"

size_t gb::bytes(size_t num)
{
    return num;
}

size_t gb::kb(size_t num)
{
    return gb::bytes(num)*1024;
}

