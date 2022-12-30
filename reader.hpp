#pragma once

#include <string>
#include <vector>
#include <map>

enum class InsturctionType : uint8_t {
    ADD
};

// FIXME: remove label from every instruction
struct Instruction
{
    std::string label;
    InsturctionType type;
    std::vector<std::string> operands;
};

class Reader {
public:
    Reader(const std::string& filename);
    std::vector<Instruction> readFile();

private:
    bool isLabel(const std::string& labelOrInstruction) const;
    Instruction retriveInsturction(const std::string& partsOfInstruction);

private:
    std::string m_programName;
    std::map<std::string, InsturctionType> m_supportedInstructions;
};