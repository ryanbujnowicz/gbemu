#include "printable.h"

std::ostream& gb::operator<<(std::ostream& out, const Printable& printable) 
{
    out << printable.toStr();
    return out;
}

