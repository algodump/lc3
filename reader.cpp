#include "reader.hpp"
#include "assembler.hpp"

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

uint8_t retrieveRegisterNumber(const std::string& lc3register)
{
    if (!lc3register.empty() && lc3register[0] == 'R') {
        return lc3register[1] - '0';
    }
    return std::numeric_limits<uint8_t>::max();
}

uint8_t retriveImmediateValue(const std::string& immediateValue)
{
    // TODO: check for overflow
    assert(immediateValue[0] == '#');
    return std::stoi(immediateValue.substr(1));
}

} // namespace

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
        }
        else if (!part.empty() && part[0] == '"') {
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
                tokens.push_back(
                    std::make_shared<OriginDerective>(to_int(operands[0])));
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
            else if (name == "ADD" || name == "AND") {
                uint8_t destinationRegister =
                    retrieveRegisterNumber(operands[0]);
                uint8_t source1Register = retrieveRegisterNumber(operands[1]);
                bool isImmediate = Assembler::isImmediate(operands[2]);
                uint8_t source2RegisterOrImmediate =
                    isImmediate ? retriveImmediateValue(operands[2])
                                : retrieveRegisterNumber(operands[2]);
                if (name == "ADD") {
                    tokens.push_back(std::make_shared<AddInstruction>(
                        destinationRegister, source1Register,
                        source2RegisterOrImmediate, isImmediate));
                }
                else {
                    tokens.push_back(std::make_shared<AndInstruction>(
                        destinationRegister, source1Register,
                        source2RegisterOrImmediate, isImmediate));
                }
            }
            else if (name.starts_with("BR")) {
                // This instruction comes in 8 falvours)
                // BRn BRzp BRz BRnp BRp BRnz BR BRnzp
                std::string conditionalCodes = name.substr(2);
                std::string labelToBranch = operands[0];
                tokens.push_back(std::make_shared<BrInstruction>(
                    conditionalCodes, labelToBranch));
            }
            else if (name == "JMP") {
                uint8_t baseRegister = retrieveRegisterNumber(operands[0]);
                tokens.push_back(
                    std::make_shared<JmpInsturction>(baseRegister));
            }
            else if (name == "RET") {
                tokens.push_back(std::make_shared<RetInstruction>());
            }
            else if (name == "JSR") {
                auto labelOrImmediateOffset = operands[0];
                tokens.push_back(
                    std::make_shared<JsrInstruction>(labelOrImmediateOffset));
            }
            else if (name == "JSRR") {
                uint8_t baseRegister = retrieveRegisterNumber(operands[0]);
                tokens.push_back(
                    std::make_shared<JsrrInstruction>(baseRegister));
            }
            else if (name == "LD") {
                uint8_t destinationRegister =
                    retrieveRegisterNumber(operands[0]);
                std::string labelOrImmediateOffset = operands[1];
                tokens.push_back(std::make_shared<LdInstruction>(
                    destinationRegister, labelOrImmediateOffset));
            }
            else if (name == "LDI") {
                uint8_t destinationRegister =
                    retrieveRegisterNumber(operands[0]);
                std::string labelOrImmediateOffset = operands[1];
                tokens.push_back(std::make_shared<LdiInsturction>(
                    destinationRegister, labelOrImmediateOffset));
            }
            else if (name == "LDR") {
                uint8_t destinationRegister =
                    retrieveRegisterNumber(operands[0]);
                uint8_t baseRegister = retrieveRegisterNumber(operands[1]);
                std::string labelOrImmediateOffset = operands[2];
                tokens.push_back(std::make_shared<LdrInstruction>(
                    baseRegister, destinationRegister, labelOrImmediateOffset));
            }
            else if (name == "LEA") {
                uint8_t destinationRegister =
                    retrieveRegisterNumber(operands[0]);
                std::string labelOrImmediateOffset = operands[1];
                tokens.push_back(std::make_shared<LeaInstruction>(
                    destinationRegister, labelOrImmediateOffset));
            }
            else if (name == "ST") {
                uint8_t sourceRegister = retrieveRegisterNumber(operands[0]);
                std::string labelOrImmediateOffset = operands[1];
                tokens.push_back(std::make_shared<StInstruction>(
                    sourceRegister, labelOrImmediateOffset));
            }
            else if (name == "STI") {
                uint8_t sourceRegister = retrieveRegisterNumber(operands[0]);
                std::string labelOrImmediateOffset = operands[1];
                tokens.push_back(std::make_shared<StiInstruction>(
                    sourceRegister, labelOrImmediateOffset));
            }
            else if (name == "NOT") {
                uint8_t destinationRegister =
                    retrieveRegisterNumber(operands[0]);
                uint8_t sourceRigistere = retrieveRegisterNumber(operands[1]);
                tokens.push_back(std::make_shared<NotInstruction>(
                    destinationRegister, sourceRigistere));
            }
            else if (name == "RTI") {
                tokens.push_back(std::make_shared<RtiInstruction>());
            } else if (name == "TRAP") {
                uint8_t trapVector = to_int(operands[0]);
                tokens.push_back(std::make_shared<TrapInstruction>(trapVector));
            }

            offsetFromOrigin++;
        }
    }
    return tokens;
}
