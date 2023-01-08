#include <iostream>

#include "assembler.hpp"
#include "reader.hpp"

int main() {
    Reader reader("..\\..\\tests\\playground.lc3");
    auto instructions = reader.readFile();

    Assembler assembler(instructions);
    Writer writer("..\\..\\..\\out.l3_bin");

    assembler.gnenerate(writer);
    return 0;
}