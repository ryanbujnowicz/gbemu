#ifndef GB_UNITS_H
#define GB_UNITS_H

namespace gb {

typedef unsigned char Byte;

size_t bytes(size_t num)
{
    return num;
}

size_t kb(size_t num)
{
    return 1024*num;
}

}

#endif

