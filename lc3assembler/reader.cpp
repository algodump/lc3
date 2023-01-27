#include "reader.hpp"
#include "assembler.hpp"

#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {
void checkOperands(const std::string& currentLine,
                   uint8_t expectedNumberOfOperands,
                   uint8_t actualNumberOfOperands)
{
    if (expectedNumberOfOperands != actualNumberOfOperands) {
        throw std::runtime_error(std::format(
            "Wrong number of operands for `{}` expected {} operands, got {}",
            currentLine, expectedNumberOfOperands, actualNumberOfOperands));
    }
}

uint16_t retrieveNumber(const std::string& number)
{
    if (number[0] == 'x' || number[0] == 'X') {
        std::string stoiComptable = "0" + number;
        return std::stoi(stoiComptable, nullptr, 16);
    }
    else if (number[0] == '#') {
        return std::stoi(number.substr(1));
    }
    else {
        throw std::runtime_error(std::format(
            "Wrong number format: number should start form `#` or `x`"));
    }
}

uint8_t retrieveRegisterNumber(const std::string& lc3register)
{
    if (lc3register.size() == 2 && lc3register[0] == 'R' &&
        ((lc3register[1] - '0') >= 0) && ((lc3register[1] - '0') <= 7)) {
        return lc3register[1] - '0';
    }
    else {
        throw std::runtime_error(
            std::format("Wrong register format: got: {}, expected: {}",
                        lc3register, "R{0, 1, 2, 3, 4, 5, 6, 7}"));
    }
}
} // namespace

Reader::Reader(const std::string& filename) : m_programName(filename) {}

InstructionToken Reader::parseInsturction(const std::string& insturction)
{
    std::istringstream iss(insturction);
    auto readInstructionName = [&iss]() {
        std::string instructionName;
        std::getline(iss >> std::ws, instructionName, ' ');
        return instructionName;
    };

    std::string instructionName = readInstructionName();
    std::string label;
    // NOTE: label is optional
    if (!SupportedInsturctions::isAssemblyKeyword(instructionName)) {
        label = instructionName;
        instructionName = readInstructionName();
    }

    // TODO: make this vector of ints or someting
    std::vector<std::string> operands;
    for (std::string operand; std::getline(iss >> std::ws, operand, ',');) {
        if (!operand.empty() && operand[0] != '"') {
            // NOTE: remove all of the trailing spaces if any
            operands.push_back(operand.substr(0, operand.find_first_of(' ')));
        }
        else if (!operand.empty() && operand[0] == '"') {
            operands.push_back(
                operand.substr(1, operand.find_first_of('"', 1) - 1));
            break;
        }
    }
    return InstructionToken{
        .label = label, .name = instructionName, .operands = operands};
}

std::vector<InstructionWithAddress> Reader::readFile()
{
    std::vector<InstructionWithAddress> tokens;
    try {
        std::ifstream ifs(m_programName);
        if (!ifs.is_open()) {
            std::runtime_error(
                std::format("Couldn't open file: `{}`", m_programName));
        }

        uint16_t pc = 0;
        for (std::string currentLine;
             std::getline(ifs >> std::ws, currentLine);) {
            // NOTE: skip lines that start with comments
            if (!currentLine.empty() && currentLine[0] != ';') {
                std::string currentLineWithoutComment =
                    currentLine.substr(0, currentLine.find(';'));
                auto&& [label, name, operands] =
                    parseInsturction(currentLineWithoutComment);
                // add labels to the symbol table
                if (!label.empty()) {
                    SymbolTable::the().add(label, pc);
                }

                // parse assembly directives
                if (name[0] == '.') {
                    if (name == ".ORIG") {
                        checkOperands(currentLine, 1, operands.size());
                        tokens.push_back(
                            {.instruction = std::make_shared<OriginDerective>(
                                 retrieveNumber(operands[0])),
                             .address = pc});
                    }
                    else if (name == ".FILL") {
                        checkOperands(currentLine, 1, operands.size());
                        tokens.push_back(
                            {.instruction = std::make_shared<FillDerective>(
                                 retrieveNumber(operands[0])),
                             .address = pc});
                        pc += 1;
                    }
                    else if (name == ".BLKW") {
                        checkOperands(currentLine, 1, operands.size());
                        auto numberOfMemoryLocations =
                            retrieveNumber(operands[0]);
                        tokens.push_back(
                            {.instruction = std::make_shared<BlkwDerective>(
                                 numberOfMemoryLocations),
                             .address = pc});
                        pc += numberOfMemoryLocations;
                    }
                    else if (name == ".STRINGZ") {
                        checkOperands(currentLine, 1, operands.size());
                        auto lc3String = operands[0];
                        tokens.push_back(
                            {.instruction =
                                 std::make_shared<StringDerective>(lc3String),
                             .address = pc});
                        pc += lc3String.size() + 1;
                    }
                    else if (name == ".END") {
                        checkOperands(currentLine, 0, operands.size());
                        tokens.push_back(
                            {.instruction = std::make_shared<EndDerective>(),
                             .address = pc});
                    }
                }
                else {
                    // parse normal instructions
                    if (name == "ADD" || name == "AND") {
                        checkOperands(currentLine, 3, operands.size());
                        uint8_t destinationRegister =
                            retrieveRegisterNumber(operands[0]);
                        uint8_t source1Register =
                            retrieveRegisterNumber(operands[1]);
                        bool isImmediate = Assembler::isImmediate(operands[2]);
                        uint8_t source2RegisterOrImmediate =
                            isImmediate ? retrieveNumber(operands[2])
                                        : retrieveRegisterNumber(operands[2]);
                        if (name == "ADD") {
                            tokens.push_back(
                                {.instruction =
                                     std::make_shared<AddInstruction>(
                                         destinationRegister, source1Register,
                                         source2RegisterOrImmediate,
                                         isImmediate),
                                 .address = pc});
                        }
                        else {
                            tokens.push_back(
                                {.instruction =
                                     std::make_shared<AndInstruction>(
                                         destinationRegister, source1Register,
                                         source2RegisterOrImmediate,
                                         isImmediate),
                                 .address = pc});
                        }
                    }
                    else if (name.starts_with("BR")) {
                        // This instruction comes in 8 falvours)
                        // BRn BRzp BRz BRnp BRp BRnz BR BRnzp
                        checkOperands(currentLine, 1, operands.size());
                        std::string conditionalCodes = name.substr(2);
                        std::string labelToBranch = operands[0];
                        tokens.push_back(
                            {.instruction = std::make_shared<BrInstruction>(
                                 conditionalCodes, labelToBranch),
                             .address = pc});
                    }
                    else if (name == "JMP") {
                        checkOperands(currentLine, 1, operands.size());
                        uint8_t baseRegister =
                            retrieveRegisterNumber(operands[0]);
                        tokens.push_back(
                            {.instruction =
                                 std::make_shared<JmpInsturction>(baseRegister),
                             .address = pc});
                    }
                    else if (name == "RET") {
                        checkOperands(currentLine, 0, operands.size());
                        tokens.push_back(
                            {.instruction = std::make_shared<RetInstruction>(),
                             .address = pc});
                    }
                    else if (name == "JSR") {
                        checkOperands(currentLine, 1, operands.size());
                        auto labelOrImmediateOffset = operands[0];
                        tokens.push_back(
                            {.instruction = std::make_shared<JsrInstruction>(
                                 labelOrImmediateOffset),
                             .address = pc});
                    }
                    else if (name == "JSRR") {
                        checkOperands(currentLine, 1, operands.size());
                        uint8_t baseRegister =
                            retrieveRegisterNumber(operands[0]);
                        tokens.push_back(
                            {.instruction = std::make_shared<JsrrInstruction>(
                                 baseRegister),
                             .address = pc});
                    }
                    else if (name == "LD") {
                        checkOperands(currentLine, 2, operands.size());
                        uint8_t destinationRegister =
                            retrieveRegisterNumber(operands[0]);
                        std::string labelOrImmediateOffset = operands[1];
                        tokens.push_back(
                            {.instruction = std::make_shared<LdInstruction>(
                                 destinationRegister, labelOrImmediateOffset),
                             .address = pc});
                    }
                    else if (name == "LDI") {
                        checkOperands(currentLine, 2, operands.size());
                        uint8_t destinationRegister =
                            retrieveRegisterNumber(operands[0]);
                        std::string labelOrImmediateOffset = operands[1];
                        tokens.push_back(
                            {.instruction = std::make_shared<LdiInsturction>(
                                 destinationRegister, labelOrImmediateOffset),
                             .address = pc});
                    }
                    else if (name == "LDR") {
                        checkOperands(currentLine, 3, operands.size());
                        uint8_t destinationRegister =
                            retrieveRegisterNumber(operands[0]);
                        uint8_t baseRegister =
                            retrieveRegisterNumber(operands[1]);
                        std::string labelOrImmediateOffset = operands[2];
                        tokens.push_back(
                            {.instruction = std::make_shared<LdrInstruction>(
                                 destinationRegister, baseRegister,
                                 labelOrImmediateOffset),
                             .address = pc});
                    }
                    else if (name == "LEA") {
                        checkOperands(currentLine, 2, operands.size());
                        uint8_t destinationRegister =
                            retrieveRegisterNumber(operands[0]);
                        std::string labelOrImmediateOffset = operands[1];
                        tokens.push_back(
                            {.instruction = std::make_shared<LeaInstruction>(
                                 destinationRegister, labelOrImmediateOffset),
                             .address = pc});
                    }
                    else if (name == "ST") {
                        checkOperands(currentLine, 2, operands.size());
                        uint8_t sourceRegister =
                            retrieveRegisterNumber(operands[0]);
                        std::string labelOrImmediateOffset = operands[1];
                        tokens.push_back(
                            {.instruction = std::make_shared<StInstruction>(
                                 sourceRegister, labelOrImmediateOffset),
                             .address = pc});
                    }
                    else if (name == "STI") {
                        checkOperands(currentLine, 2, operands.size());
                        uint8_t sourceRegister =
                            retrieveRegisterNumber(operands[0]);
                        std::string labelOrImmediateOffset = operands[1];
                        tokens.push_back(
                            {.instruction = std::make_shared<StiInstruction>(
                                 sourceRegister, labelOrImmediateOffset),
                             .address = pc});
                    }
                    else if (name == "NOT") {
                        checkOperands(currentLine, 2, operands.size());
                        uint8_t destinationRegister =
                            retrieveRegisterNumber(operands[0]);
                        uint8_t sourceRigistere =
                            retrieveRegisterNumber(operands[1]);
                        tokens.push_back(
                            {.instruction = std::make_shared<NotInstruction>(
                                 destinationRegister, sourceRigistere),
                             .address = pc});
                    }
                    else if (name == "RTI") {
                        checkOperands(currentLine, 0, operands.size());
                        tokens.push_back(
                            {.instruction = std::make_shared<RtiInstruction>(),
                             .address = pc});
                    }
                    else if (name == "STR") {
                        checkOperands(currentLine, 3, operands.size());
                        uint8_t sourceRegister =
                            retrieveRegisterNumber(operands[0]);
                        uint8_t baseRegister =
                            retrieveRegisterNumber(operands[1]);
                        std::string labelOrImmediateOffset = operands[2];
                        tokens.push_back(
                            {.instruction = std::make_shared<StrInstruction>(
                                 sourceRegister, baseRegister,
                                 labelOrImmediateOffset),
                             .address = pc});
                    }
                    else if (name == "TRAP") {
                        checkOperands(currentLine, 1, operands.size());
                        uint8_t trapVector = retrieveNumber(operands[0]);
                        tokens.push_back(
                            {.instruction =
                                 std::make_shared<TrapInstruction>(trapVector),
                             .address = pc});
                    }
                    else if (SupportedInsturctions::isTrapInstruction(name)) {
                        checkOperands(currentLine, 0, operands.size());
                        tokens.push_back(
                            {.instruction = std::make_shared<TrapInstruction>(
                                 SupportedInsturctions::getTrapCode(name)),
                             .address = pc});
                    }
                    else if (!name.empty() && label.empty()) {
                        throw std::runtime_error(
                            std::format("Unknown instruction: {}", name));
                    }
                    if (!name.empty()) {
                        pc++;
                    }
                }
            }
        }
        if (!dynamic_cast<OriginDerective*>(
                (tokens.front().instruction).get()) ||
            !dynamic_cast<EndDerective*>((tokens.back().instruction).get())) {
            throw std::runtime_error("Every lc3 assembly program should start "
                                     "with .ORIG and end with .END");
        }
    }

    catch (...) {
        throw;
    }

    return tokens;
}
