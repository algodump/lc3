#pragma once

#include <Windows.h>
#include <array>
#include <assert.h>
#include <conio.h>
#include <cstdint>
#include <signal.h>

#undef max

namespace {
// SOURCE:
// https://github.com/justinmeiners/lc3-vm/blob/master/docs/src/lc3-win.c
HANDLE hStdin = INVALID_HANDLE_VALUE;
DWORD fdwMode, fdwOldMode;

void disable_input_buffering()
{
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwOldMode);     /* save old mode */
    fdwMode = fdwOldMode ^ ENABLE_ECHO_INPUT /* no input echo */
              ^ ENABLE_LINE_INPUT;           /* return when one or
                                                more characters are available */
    SetConsoleMode(hStdin, fdwMode);         /* set new mode */
    FlushConsoleInputBuffer(hStdin);         /* clear buffer */
}

void restore_input_buffering() { SetConsoleMode(hStdin, fdwOldMode); }

uint16_t check_key()
{
    return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}

void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}
} // namespace

class Memory {
  private:
    static constexpr uint16_t START_OF_USER_PROGRAMS = 0x3000;
    static constexpr uint16_t LC3_MEMORY_CAPCITY = std::numeric_limits<uint16_t>::max();
    static constexpr uint16_t KEYBOARD_STATUS_REGISTER = 0xFE00;
    static constexpr uint16_t KEYBOARD_DATA_REGISTER = 0xFE02;
    using L3Memory = std::array<uint16_t, LC3_MEMORY_CAPCITY>;

  public:
    Memory() {}

    uint16_t operator[](uint16_t address)
    {
        if (address == KEYBOARD_STATUS_REGISTER) {
            if (check_key()) {
                m_memory[KEYBOARD_STATUS_REGISTER] = (1 << 15);
                m_memory[KEYBOARD_DATA_REGISTER] = getchar();
            }
            else {
                m_memory[KEYBOARD_STATUS_REGISTER] = 0;
            }
        }
        // assert(address >= START_OF_USER_PROGRAMS && address <=
        // MEMORY_CAPACITY);
        return m_memory[address];
    }

    void write(uint16_t address, uint16_t value)
    {
        // assert(address >= START_OF_USER_PROGRAMS && address <=
        // MEMORY_CAPACITY);
        m_memory[address] = value;
    }

  private:
    L3Memory m_memory;
};