#!/usr/bin/env python
import sys

def main(opcodeFile):
    try:
        with open(opcodeFile) as f:
            for line in f:
                (opcode, desc) = [x.strip() for x in line.split("|")]

                opcode = "0x%s" % opcode.split(" ")[0].upper()

                print "TEST_F(CPUTest, Opcode%sTest)" % opcode
                print "{"
                print "    // Testing %s" % desc
                print "    EXPECT_TRUE(false);"
                print "}"
                print ""

    except IOError as e:
        print e

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print "usage: genOpcodeBoilerplate.py <opcodeFile>"
    main(sys.argv[1])
