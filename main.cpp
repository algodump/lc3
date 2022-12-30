#include <iostream>

#include "reader.hpp"
#include "assembler.hpp"

int main() {
    Reader reader("..\\in.lc3");

    auto instructions = reader.readFile();

    Assembler assembler(instructions);
    Writer writer("..\\out.l3_bin");
    assembler.gnenerate(writer);
    return 0;
}