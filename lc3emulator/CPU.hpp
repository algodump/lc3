#pragma once

#include <string>
#include <array>

class CPU {
  public:
    CPU() = default;
    bool load(const std::string& fileToRun);
    bool emulate();

    private:
    void dumpMemory(uint16_t start, uint16_t size);

  private:
    static constexpr uint16_t MEMORY_CAPACITY = std::numeric_limits<uint16_t>::max();
    static constexpr uint8_t NUMBER_OF_REGISTERS = 8;

private:    
    std::array<uint16_t, MEMORY_CAPACITY> m_memory;
    std::array<uint8_t, NUMBER_OF_REGISTERS> m_registers;
    uint16_t m_pc;
};