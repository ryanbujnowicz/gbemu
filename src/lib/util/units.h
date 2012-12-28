#ifndef GB_UNITS_H
#define GB_UNITS_H

#include <cstddef>

namespace gb {

typedef unsigned char Word;
typedef unsigned short Dword;

size_t bytes(size_t num);
size_t kb(size_t num);

}

#endif

