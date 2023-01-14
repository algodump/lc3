#include <iostream>

#include "assembler.hpp"
#include "reader.hpp"
#include <filesystem>

// TODO: check for overflows
// TODO: discard unkown symbols
// TODO: make warrning about using out of range values
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "usage: lc3assembler <filename> " << std::endl;
        return 1;
    }

    std::string assemblyFile = argv[1];
    Reader reader(assemblyFile);
    auto instructions = reader.readFile();

    Assembler assembler(instructions);
    std::string outFileName = "out.lc3";
    Writer writer(outFileName);

    assembler.gnenerate(writer);
    return 0;
}