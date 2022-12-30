#include "reader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

Reader::Reader(const std::string &filename) : m_programName(filename) {}

bool Reader::isLabel(const std::string& labelOrInstructionTokens) const
{
    return labelOrInstructionTokens.find(":") != std::string::npos;
}

InstructionToken Reader::retriveInsturction(const std::string& partsOfInstructionToken)
{
    std::istringstream iss(partsOfInstructionToken);
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
        if (auto commentBeing = part.find_first_of(" ;"); commentBeing != std::string::npos) {
            operands.push_back(part.substr(0, commentBeing));
            break;
        }
        operands.push_back(part);
    }
    return InstructionToken{.label = label, .name = instructionName, .operands = operands};
}

std::vector<std::shared_ptr<Instruction>> Reader::readFile() 
{
    std::vector<std::shared_ptr<Instruction>> tokens;
    std::ifstream ifs(m_programName);

    for (std::string currentLine; std::getline(ifs >> std::ws, currentLine);) {
        // skip comments
        if (!currentLine.empty() && currentLine[0] != ';') {
            auto&& [label, name, operands] = retriveInsturction(currentLine);
            if (name == "ADD") {
                tokens.push_back(std::make_shared<AddInstruction>(operands));
            }
        }
    }

    return tokens;
}