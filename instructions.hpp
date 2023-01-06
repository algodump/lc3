#pragma once

#include <bitset>
#include <iostream>
#include <set>
#include <vector>

class SupportedInsturctions {
  public:
    static bool isInstruction(const std::string& maybeInsturction)
    {
        static std::set<std::string> supportedInstruction = {
            "ADD",   "AND",   "BR",    "BRn",      "BRzp", "BRz",
            "BRnp",  "BRp",   "BRnz",  "BRnzp",    "JMP",  "JSR",
            "JSRR",  "LD",    "LDI",   "LDR",      "LEA",  "NOT",
            "RET",   "RTI",   "ST",    "STI",      "STR",  "TRAP",
            ".ORIG", ".FILL", ".BLKW", ".STRINGZ", ".END"};
        return supportedInstruction.find(maybeInsturction) !=
               supportedInstruction.end();
    }
};

class InstructionBuilder {
  public:
    InstructionBuilder();
    // TODO: make bits as uint
    InstructionBuilder& set(const std::string& bits);
    InstructionBuilder& set(char bit);

    uint16_t instruction() const;

  private:
    uint8_t m_bitPointer;
    std::bitset<16> m_instruction;
};

class Instruction {
  public:
    virtual uint16_t generate() = 0;
    virtual std::string opcode() const;

    virtual ~Instruction();

  protected:
    InstructionBuilder m_assembelyInstruction;
};

class AddInstruction : public Instruction {
  public:
    // FIXME: parse the operands during the reading stage,
    //        becaues vector<string> is to vague
    AddInstruction(const std::vector<std::string>& operands);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    std::vector<std::string> m_operands;
};

class LoadInstruction : public Instruction {
  public:
    LoadInstruction(const std::vector<std::string>& operands);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    std::vector<std::string> m_operands;
};

class AndInstruction : public Instruction {
  public:
    AndInstruction(const std::vector<std::string>& operands);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    std::vector<std::string> m_operands;
};

class BrInstruction : public Instruction {
  public:
    BrInstruction(const std::string& conditionalCodes,
                  const std::string& label);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    std::string m_conditionalCodes;
    std::string m_label;
};

class JmpInsturction : public Instruction {
  public:
    JmpInsturction(uint16_t baseRegister);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    uint16_t m_baseRegister;
};

class RetInstruction : public Instruction {
  public:
    RetInstruction() = default;
    uint16_t generate() override;
    std::string opcode() const override;
};

class OriginDerective : public Instruction {
  public:
    OriginDerective(uint16_t origin);
    uint16_t generate() override;

  private:
    uint16_t m_origin;
};

class FillDerective : public Instruction {
  public:
    FillDerective(uint16_t value);
    uint16_t generate() override;

  private:
    uint16_t m_value;
};

class BlkwDerective : public Instruction {
  public:
    BlkwDerective(uint16_t numberOfMemoryLocations);
    uint16_t generate() override;

    uint16_t getNumberOfMemoryLocations() const;

  private:
    uint16_t m_numberOfMemoryLocations;
};

class StringDerective : public Instruction {
  public:
    StringDerective(const std::string& str);
    uint16_t generate() override;

    std::string getStringToWrite() const;

  private:
    std::string m_stringToWrite;
};

class EndDerective : public Instruction {
  public:
    EndDerective() = default;
    uint16_t generate() override;
};