#pragma once

#include "instructions.hpp"

#include <string>
#include <vector>
#include <map>
#include <memory>


// FIXME: remove label from every InstructionToken
struct InstructionToken
{
    std::string label;
    std::string name;
    std::vector<std::string> operands;
};

class Reader {
public:
    Reader(const std::string& filename);
    std::vector<std::shared_ptr<Instruction>> readFile();

private:
    bool isLabel(const std::string& labelOrInstructionToken) const;
    InstructionToken retriveInsturction(const std::string& partsOfInstructionToken);

private:
    std::string m_programName;
};