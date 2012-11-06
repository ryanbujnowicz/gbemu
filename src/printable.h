#ifndef GB_PRINTABLE_H
#define GB_PRINTABLE_H

#include <ostream>
#include <string>

namespace gb {

/**
 * Defines an interface which when filled enables the object to be printed.
 */
class Printable
{
public:
    virtual std::string toStr() const = 0;
};

/**
 * Prints any Printable object to the given output stream.
 */
std::ostream& operator<<(std::ostream& out, const Printable& printable);

}

#endif

