#include "mmu.h"
using namespace gb;

#include <cassert>

MMU::MMU()
{
}

MMU::~MMU()
{
}

bool MMU::validAddress(size_t address) const 
{
    for (const MapEntry& e : _entries) {
        if (e.localRange.contains(address)) {
            return true;
        }
    }
    return false;
}

gb::Word& MMU::operator[](size_t address)
{
    for (const MapEntry& e : _entries) {
        if (e.localRange.contains(address)) {
            size_t diff = address - e.localRange.min();
            assert(e.target);
            return (*e.target)[e.targetRange.min() + diff];
        }
    }
    assert(false && "MMU: unmapped memory access attempted");
}

void MMU::map(Addressable* target, gb::Range targetRange, gb::Range localRange)
{
#ifndef NDEBUG
    for (const MapEntry& e : _entries) {
        assert(!e.localRange.contains(localRange.min()) && 
               !e.localRange.contains(localRange.max()));
    }
#endif

    MapEntry newEntry(target, targetRange, localRange);
    _entries.push_back(newEntry);
}

