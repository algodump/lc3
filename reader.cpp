#include "reader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

Reader::Reader(const std::string &filename) : m_programName(filename),
                                              m_supportedInstructions{{"ADD", InsturctionType::ADD}} {}


bool Reader::isLabel(const std::string& labelOrInstructions) const
{
    return m_supportedInstructions.find(labelOrInstructions) == m_supportedInstructions.end();
}

Instruction Reader::retriveInsturction(const std::string& partsOfInstruction)
{
    std::istringstream iss(partsOfInstruction);
    std::string label;
    std::string instructionName;

    std::getline(iss, label, ' ');
    if (isLabel(label)) {
        std::getline(iss, instructionName, ' ');
    } else {
        instructionName = label;
        label = "";
    }

    std::vector<std::string> operands;
    for (std::string part; std::getline(iss >> std::ws, part, ',');) {
        if (auto commentBeing = part.find(';'); commentBeing != std::string::npos) {
            operands.push_back(part.substr(0, commentBeing));
            break;
        }
        operands.push_back(part);
    }
    return Instruction{.label = label, .type = m_supportedInstructions[instructionName], .operands = operands};
}

std::vector<Instruction> Reader::readFile() 
{
    std::vector<Instruction> tokens;
    std::ifstream ifs(m_programName);

    for (std::string currentLine; std::getline(ifs >> std::ws, currentLine);) {
        // skip comments
        if (!currentLine.empty() && currentLine[0] != ';') {
            tokens.push_back(retriveInsturction(currentLine));
        }
    }

    return tokens;
}