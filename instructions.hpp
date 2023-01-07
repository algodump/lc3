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
    AddInstruction(uint8_t destinationRegister, uint8_t source1Register,
                   uint8_t source2RegisterOrImmediate, bool isImmediate);

    uint16_t generate() override;
    std::string opcode() const override;

  private:
    uint8_t m_destinationRegister;
    uint8_t m_source1Register;
    union {
        uint8_t m_source2Register;
        uint8_t m_immediateValue;
    };
    bool m_isImmediate;
};

class AndInstruction : public Instruction {
  public:
    AndInstruction(uint8_t destinationRegister, uint8_t source1Register,
                   uint8_t source2RegisterOrImmediate, bool isImmediate);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    uint8_t m_destinationRegister;
    uint8_t m_source1Register;
    union {
        uint8_t m_source2Register;
        uint8_t m_immediateValue;
    };
    bool m_isImmediate;
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
    JmpInsturction(uint8_t baseRegister);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    uint8_t m_baseRegister;
};

class RetInstruction : public Instruction {
  public:
    RetInstruction() = default;
    uint16_t generate() override;
    std::string opcode() const override;
};

class JsrInstruction : public Instruction {
  public:
    JsrInstruction(const std::string& labelOrOffset);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    std::string m_labelOrOffset;
};

class JsrrInstruction : public Instruction {
  public:
    JsrrInstruction(uint8_t baseRegister);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    uint8_t m_baseRegister;
};

class LdInstruction : public Instruction {
  public:
    LdInstruction(uint8_t destinationRegister, const std::string& m_labelOrOffset);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    uint8_t m_destinationRegister;
    std::string m_labelOrOffset;
};

class LdiInsturction : public Instruction {
  public:
    LdiInsturction(uint8_t destinationRegister, const std::string& labelOrOffset);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    uint8_t m_destinationRegister;
    std::string m_labelOrOffset;
};

class LdrInstruction : public Instruction {
  public:
    LdrInstruction(uint8_t destinationRegister, uint8_t baseRegister,
                   const std::string& labelOrOffset);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    uint8_t m_destinationRegister;
    uint8_t m_baseRegister;
    std::string m_labelOrOffset;
};

// TODO: make this class template
class LeaInstruction : public Instruction {
  public:
    LeaInstruction(uint8_t destinationRegister, const std::string& m_labelOrOffset);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    uint8_t m_destinationRegister;
    std::string m_labelOrOffset;
};

class StInstruction : public Instruction {
  public:
    StInstruction(uint8_t sourceRegister, const std::string& m_labelOrOffset);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    uint8_t m_sourceRegister;
    std::string m_labelOrOffset;
};

class StiInstruction : public Instruction {
  public:
    StiInstruction(uint8_t sourceRegister, const std::string& m_labelOrOffset);
    uint16_t generate() override;
    std::string opcode() const override;

  private:
    uint8_t m_sourceRegister;
    std::string m_labelOrOffset;
};

class NotInstruction : public Instruction {
  public:
    NotInstruction(uint8_t destinationRegister, uint8_t sourceRegister);

    uint16_t generate() override;
    std::string opcode() const override;

  private:
    uint8_t m_destinationRegister;
    uint8_t m_sourceRegister;
};

class RtiInstruction : public Instruction {
  public:
    RtiInstruction() = default;
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