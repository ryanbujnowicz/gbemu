#ifndef GB_MMU_H
#define GB_MMU_H

#include <vector>

#include "addressable.h"
#include "units.h"
#include "util.h"

namespace gb {

/**
 * A MMU (memory mapper unit) provides a virtual memory abstraction over multiple addressable units.
 *
 * This takes the form of an address mapping range.
 */
class MMU : public Addressable
{
public:
    MMU();
    ~MMU();

    virtual bool validAddress(size_t address) const override;
    virtual gb::Word& operator[](size_t address) override;

    /**
     * Caller retains ownership of target.
     *
     * targetRange and localRange must be of the same size. No two local ranges
     * should overlap.
     */
    void map(Addressable* target, gb::Range targetRange, gb::Range localRange);

private:
    struct MapEntry
    {
        MapEntry(Addressable* target, const gb::Range& targetRange, const gb::Range& localRange) : 
            target(target),
            targetRange(targetRange),
            localRange(localRange)
        {
        }

        Addressable* target;
        gb::Range targetRange;   
        gb::Range localRange;   
    };
    std::vector<MapEntry> _entries;
};

}

#endif

