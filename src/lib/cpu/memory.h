#ifndef GB_MEMORY_H
#define GB_MEMORY_H

#include <cassert>
#include <sstream>
#include <string>

#include "addressable.h"
#include "util/units.h"

namespace gb {

/**
 * Represents an addressable piece of memory capable of storing an arbitrary
 * number of bytes.
 */
class Memory : public gb::Addressable
{
public:
    
    Memory(size_t size) :
        _mem(new gb::Byte[size]),
        _size(size)
    {
    }

    ~Memory()
    {
        delete[] _mem;
    }

    virtual gb::Byte& operator[](size_t address) override
    {
        assert(isValidAddress(address));
        return _mem[address];
    }

    virtual bool isValidAddress(size_t address) const override
    {
        // Address will always be >= 0
        return address < _size;
    }

    size_t size() const
    {
        return _size;
    }

protected:

    gb::Byte* _mem;
    size_t _size;

};

}

#endif

