#include "reader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace {
uint16_t to_int(const std::string& number)
{
    if (number[0] == '0' && number[1] == 'x') {
        return std::stoi(number, nullptr, 16);
    }
    return std::stoi(number);
}
}
Reader::Reader(const std::string& filename) : m_programName(filename) {}

InstructionToken Reader::parseInsturction(const std::string& insturction)
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
        if (auto commentBeing = part.find_first_of(";");
            commentBeing != std::string::npos && part[0] != '"') {
            operands.push_back(part.substr(0, commentBeing));
            break;
        } else if (!part.empty() && part[0] == '"') {
            operands.push_back(part.substr(1, part.find_first_of('"', 1) - 1));
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

    uint16_t offsetFromOrigin = 0;
    for (std::string currentLine; std::getline(ifs >> std::ws, currentLine);) {
        // skip comments
        if (!currentLine.empty() && currentLine[0] != ';') {
            auto&& [label, name, operands] = parseInsturction(currentLine);
            // add labels to the symbol table
            if (!label.empty()) {
                SymbolTable::the().add(label, offsetFromOrigin);
            }

            // parse assembly directives
            if (name == ".ORIG") {
                tokens.push_back(std::make_shared<OriginDerective>(
                    to_int(operands[0])));
            }
            else if (name == ".FILL") {
                tokens.push_back(
                    std::make_shared<FillDerective>(to_int(operands[0])));
            }
            else if (name == ".BLKW") {
                tokens.push_back(
                    std::make_shared<BlkwDerective>(to_int(operands[0])));
            }
            else if (name == ".STRINGZ") {
                tokens.push_back(
                    std::make_shared<StringDerective>(operands[0]));
            }
            else if (name == ".END") {
                tokens.push_back(std::make_shared<EndDerective>());
            }

            // parse normal instructions
            else if (name == "ADD") {
                tokens.push_back(std::make_shared<AddInstruction>(operands));
            } else if (name == "AND") {
                tokens.push_back(std::make_shared<AndInstruction>(operands));
            } else if (name.starts_with("BR")) {
                // This instruction comes in 8 falvours)
                // BRn BRzp BRz BRnp BRp BRnz BR BRnzp
                auto conditionalCodes = name.substr(2);
                auto label = operands[0];
                tokens.push_back(std::make_shared<BrInstruction>(conditionalCodes, label));
            } else if (name == "JMP") {
                auto baseRegister = operands[0][1] - '0';
                tokens.push_back(std::make_shared<JmpInsturction>(baseRegister));
            } else if (name == "RET") {
                tokens.push_back(std::make_shared<RetInstruction>());
            } else if (name == "JSR") {
                auto label = operands[0];
                tokens.push_back(std::make_shared<JsrInstruction>(label));
            } else if (name == "JSRR") {
                auto baseRegister = operands[0][1] - '0';
                tokens.push_back(std::make_shared<JsrrInstruction>(baseRegister));
            }
            else if (name == "LD") {
                tokens.push_back(std::make_shared<LoadInstruction>(operands));
            }
            offsetFromOrigin++;
        }
    }
    return tokens;
}
