#ifndef GB_ADDRESSABLE_H
#define GB_ADDRESSABLE_H

#include "units.h"

namespace gb {

class Addressable
{
public:
    virtual gb::Byte& operator[](size_t address) = 0;
    virtual bool validAddress(size_t address) = 0;
};

}

#endif

