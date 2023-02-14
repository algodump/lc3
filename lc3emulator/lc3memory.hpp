#pragma once

#include <array>
#include <assert.h>

#include <cstdint>
#include <fmt/core.h>
#include <signal.h>

#ifdef WIN32
#include <conio.h>
#include <Windows.h>
#undef max
#endif

namespace {
// SOURCE:
// https://github.com/justinmeiners/lc3-vm/blob/master/docs/src/lc3-win.c
#ifdef WIN32
HANDLE hStdin = INVALID_HANDLE_VALUE;
DWORD fdwMode, fdwOldMode;
#endif

void disable_input_buffering()
{
    #ifdef WIN32
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwOldMode);     /* save old mode */
    fdwMode = fdwOldMode ^ ENABLE_ECHO_INPUT /* no input echo */
              ^ ENABLE_LINE_INPUT;           /* return when one or
                                                more characters are available */
    SetConsoleMode(hStdin, fdwMode);         /* set new mode */
    FlushConsoleInputBuffer(hStdin);         /* clear buffer */
    #endif
}

void restore_input_buffering() 
{ 
    #ifdef WIN32
    SetConsoleMode(hStdin, fdwOldMode);
    #endif 
}

uint16_t check_key()
{
    #ifdef WIN32
    return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
    #else 
    return 0;
    #endif
}

void handle_interrupt(int signal)
{
    #ifdef WIN32
    restore_input_buffering();
    printf("\n");
    exit(-2);
    #endif
}
} // namespace

class Memory {
  private:
    static constexpr uint16_t START_OF_USER_PROGRAMS = 0x3000;
    static constexpr uint16_t LC3_MEMORY_CAPCITY =
        std::numeric_limits<uint16_t>::max();
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
        else if (address < START_OF_USER_PROGRAMS) {
            throw std::runtime_error(
                fmt::format("Illegal memory access at address: {}", address));
        }
        return m_memory[address];
    }

    void write(uint16_t address, uint16_t value)
    {
        if (address < START_OF_USER_PROGRAMS) {
            throw std::runtime_error(
                fmt::format("Illegal memory write at address: {}", address));
        }
        m_memory[address] = value;
    }

  private:
    L3Memory m_memory;
};