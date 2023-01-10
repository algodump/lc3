#include <iostream>

#include "assembler.hpp"
#include "reader.hpp"
#include <filesystem>

int main() {
    Reader reader("lc3assembler\\tests\\playground.lc3");
    auto instructions = reader.readFile();

    Assembler assembler(instructions);
    Writer writer("out.l3_bin");

    assembler.gnenerate(writer);
    return 0;
}