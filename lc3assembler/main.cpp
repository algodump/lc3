#include <exception>
#include <filesystem>
#include <fmt/core.h>
#include <iostream>

#include "assembler.hpp"
#include "reader.hpp"

// TODO: make warrning about using out of range values

int main(int argc, char* argv[])
{
    std::string outFileName = "out.lc3";
    if (argc < 2) {
        std::cout << "usage: lc3assembler <filename> [-o output file nanme] " << std::endl;
        return 1;
    } else if (argc == 4) {
        std::string outputFlag = argv[2];
        std::string outputFilenameParameter = argv[3];
        if (outputFlag == "-o" && !outputFilenameParameter.empty()) {
            outFileName = outputFilenameParameter;
        }
    }

    try {
        std::string assemblyFile = argv[1];
        Reader reader(assemblyFile);
        auto instructions = reader.readFile();

        Assembler assembler(instructions);
        Writer writer(outFileName);

        assembler.gnenerate(writer);
        std::cout << fmt::format("Writing assembler output to: `{}`\n",
                                 outFileName);
    }
    catch (std::exception e) {
        std::cout << fmt::format("LC3 ASSEMBLER ERROR: {}\n", e.what());
    }

    return 0;
}