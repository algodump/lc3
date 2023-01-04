#pragma once

#include "instructions.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

// FIXME: remove label from every InstructionToken
struct InstructionToken {
    std::string label;
    std::string name;
    std::vector<std::string> operands;
};

class SymbolTable {
  public:
    static SymbolTable& the()
    {
        static SymbolTable table;
        return table;
    }

    void add(const std::string& label, uint16_t offset) 
    {
        m_labelsOffset.insert({label, offset});
    }

    uint16_t get(const std::string& label) const
    {
      return m_labelsOffset.at(label);
    }


    SymbolTable(const SymbolTable&) = delete;
    SymbolTable& operator=(SymbolTable&) = delete;

  private:
    SymbolTable() = default;

  private:
    std::map<std::string, uint16_t> m_labelsOffset;
};

class Reader {
  public:
    Reader(const std::string& filename);
    std::vector<std::shared_ptr<Instruction>> readFile();

  private:
    InstructionToken
    parseInsturction(const std::string& partsOfInstructionToken);

  private:
    std::string m_programName;
};