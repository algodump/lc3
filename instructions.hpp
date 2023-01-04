#pragma once

#include <iostream>
#include <vector>
#include <bitset>
#include <set>

class SupportedInsturctions {
  public:
    static bool isInstruction(const std::string& maybeInsturction) {
        static std::set<std::string> supportedInstruction = {
            "ADD",
            "AND",
            "BR",
            "JMP",
            "JSR",
            "JSRR",
            "LD",
            "LDI",
            "LDR",
            "LEA",
            "NOT",
            "RET",
            "RTI",
            "ST",
            "STI",
            "STR",
            "TRAP"
        };
        return supportedInstruction.find(maybeInsturction) != supportedInstruction.end();
    }
};

class InstructionBuilder {
public:
    InstructionBuilder();
    // TODO: make bits as uint
    InstructionBuilder &set(const std::string &bits);

    uint16_t instruction() const;
private:
    uint8_t m_bitPointer;
    std::bitset<16> m_instruction;
};

class Instruction
{
public:
    virtual uint16_t generate() = 0;
    virtual ~Instruction();

protected:
    InstructionBuilder m_assembelyInstruction;
};

class AddInstruction : public Instruction
{
public:
    AddInstruction(const std::vector<std::string>& operands);
    uint16_t generate() override;

private:
    bool isImmediate();

private:
    std::vector<std::string> m_operands;
};

class LoadInstruction : public Instruction {
  public:
    LoadInstruction(const std::vector<std::string>& operands);
    uint16_t generate() override;

  private:
    std::vector<std::string> m_operands;
};
