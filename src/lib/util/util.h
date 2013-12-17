#ifndef GB_UTIL_H
#define GB_UTIL_H

#include <cassert>
#include <cstdlib>
#include <limits>
#include <ostream>
#include <sstream>

#include "units.h"
#include "printable.h"

namespace gb {

class Range : public Printable
{
public:
    Range(size_t min, size_t max) :
        _min(min),
        _max(max)
    {
        assert(min <= max);
    }

    bool contains(size_t val) const
    {
        return val >= _min && val <= _max;
    }

    size_t min() const
    {
        return _min;
    }

    size_t max() const
    {
        return _max;
    }

    //virtual std::string toStr() const override
    virtual std::string toStr() const override
    {
        std::stringstream ss;
        ss << "gb::Range(" << _min << "," << _max << ")";
        return ss.str();
    }

    std::ostream& operator<<(std::ostream& out) const
    {
        out << toStr() << std::endl;
        return out;
    }

private:
    size_t _min;
    size_t _max;
};

std::string toStr(gb::Byte val);
std::string toStr(gb::Word val);

template <typename T>
T toHex(const std::string& s)
{
    std::stringstream ss;
    ss << std::hex << s;

    int x;
    ss >> x;

    assert(x >= std::numeric_limits<T>::min() && x <= std::numeric_limits<T>::max());
    return static_cast<T>(x);
}

/**
 * Converts i to a twos-complement representation.
 */
// TODO: encode/decode
gb::Byte toSigned8(int i);
gb::Word toSigned16(int i);

int toInt8(gb::Byte val);
int toInt16(gb::Word val);

}

#endif

