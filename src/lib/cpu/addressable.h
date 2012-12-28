#ifndef GB_ADDRESSABLE_H
#define GB_ADDRESSABLE_H

#include "units.h"

namespace gb {

/**
 * An interface defining an addressable model on an object.
 *
 * A gb::Word is defined as the smallest addressable value.
 */
class Addressable
{
public:
    virtual gb::Word& operator[](size_t address) = 0;
    virtual bool isValidAddress(size_t address) const = 0;
};

}

#endif

