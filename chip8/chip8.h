#pragma once
#include <cstdint>
#include <stack>
#include <array>

struct Quirks {
    bool emulate_old_shift { false };
    bool emulate_buggy_jump_offset { false };
    bool emulate_old_load_store { false };
};

struct Chip8 {
    static constexpr std::array<uint8_t, 16 * 5> FONT_DATA = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    uint16_t width { 64 };
    uint16_t height { 32 };

    std::array<uint8_t, 4096> ram { };
    std::array<bool, 64 * 32> vram { };
    std::array<bool, 16> input { };
    std::array<uint8_t, 16> v_registers { };
    uint16_t i_register { 0x0 };
    uint8_t delay_timer { 0 };
    uint8_t sound_timer { 0 };
    uint16_t pc { 0x200 };
    std::stack<uint16_t> pc_stack {};
    Quirks quirks {};
};

bool load_rom(Chip8& chip8, const std::string& path);
void init(Chip8& chip8);
void run(Chip8& chip8);
void update_timers(Chip8& chip8);
void math_instructions(Chip8& chip8, uint8_t n, uint8_t x, uint8_t y);
void f_instructions(Chip8& chip8, uint8_t nn, uint8_t x, uint8_t y);
void draw(Chip8& chip8, uint16_t x, uint16_t y, uint16_t height);
