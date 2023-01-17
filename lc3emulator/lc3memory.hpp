#pragma once

#include <array>
#include <cstdint>
#include <assert.h>

class Memory {
  private:
    static constexpr uint16_t START_OF_USER_PROGRAMS = 0x3000;
    static constexpr uint16_t MEMORY_CAPACITY =
        std::numeric_limits<uint16_t>::max();
    using Data = std::array<int16_t, MEMORY_CAPACITY>;

  public:
    Memory() = default;

    int16_t operator[](uint16_t address)
    {
        assert(address >= START_OF_USER_PROGRAMS && address <= MEMORY_CAPACITY);
        return m_memory[address];
    }

    void write(uint16_t address, int16_t value)
    {
        assert(address >= START_OF_USER_PROGRAMS && address <= MEMORY_CAPACITY);
        m_memory[address] = value;
    }

  private:
    Data m_memory;
};