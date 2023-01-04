#include "reader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

Reader::Reader(const std::string& filename) : m_programName(filename) {}

InstructionToken
Reader::parseInsturction(const std::string& insturction)
{
    std::istringstream iss(insturction);
    std::string instructionName;
    std::string label;
    
    // NOTE: label is optional
    iss >> instructionName;
    if (!SupportedInsturctions::isInstruction(instructionName)) {
        label = instructionName;
        iss >> instructionName;
    }

    // TODO: make this vector of ints or someting
    std::vector<std::string> operands;
    for (std::string part; std::getline(iss >> std::ws, part, ',');) {
        if (auto commentBeing = part.find_first_of(" ;");
            commentBeing != std::string::npos) {
            operands.push_back(part.substr(0, commentBeing));
            break;
        }
        operands.push_back(part);
    }
    return InstructionToken{
        .label = label, .name = instructionName, .operands = operands};
}

std::vector<std::shared_ptr<Instruction>> Reader::readFile()
{
    std::vector<std::shared_ptr<Instruction>> tokens;
    std::ifstream ifs(m_programName);

    std::string currentLine;
    std::getline(ifs >> std::ws, currentLine);
    uint16_t offsetFromOrigin = 0;

    for (; std::getline(ifs >> std::ws, currentLine);) {
        // skip comments
        if (!currentLine.empty() && currentLine[0] != ';') {
            auto&& [label, name, operands] = parseInsturction(currentLine);
            if (!label.empty()) {
                SymbolTable::the().add(label, offsetFromOrigin);
            }
            if (name == "ADD") {
                tokens.push_back(std::make_shared<AddInstruction>(operands));
            } else if (name == "LD"){
                tokens.push_back(std::make_shared<LoadInstruction>(operands));
            }
            offsetFromOrigin++;
        }
    }
    return tokens;
}
