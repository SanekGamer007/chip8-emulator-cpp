#pragma once
#include <cstdint>
#include <stack>
#include <array>
#include <string>
#include <random>

class Chip8 {

public:
    struct Quirks {
        bool emulate_old_shift { false };
        bool emulate_buggy_jump_offset { true };
        bool emulate_old_load_store { false };
        bool emulate_vf_reset { false };
        bool emulate_vblank_wait { false };
        bool emulate_clear_on_mode_switch { true };
    };

    bool vblank { false };
    int audio_frequency = 480;
    uint8_t delay_timer { 0 };
    uint8_t sound_timer { 0 };
    std::array<bool, 16> input { };
    Quirks quirks {};
    std::mt19937 rng;
    std::uniform_int_distribution<uint16_t> dist{0, 255};

    bool load_rom(const std::string& path);
    virtual void init();
    virtual void run();
    void update_timers();
    [[nodiscard]] uint16_t get_opcode() const;

    [[nodiscard]] uint16_t get_width() const {
        return width;
    }
    [[nodiscard]] uint16_t get_height() const {
        return height;
    }
    [[nodiscard]] const std::array<uint8_t, 128 * 64>& get_vram() const {
        return vram;
    }

    virtual ~Chip8() = default;


protected:
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

    std::array<uint8_t, 4096> ram { };
    std::array<uint8_t, 128 * 64> vram { };
    std::array<uint8_t, 16> v_registers { };
    uint16_t i_register { 0x0 };
    uint16_t pc { 0x200 };
    std::array<uint16_t, 255> pc_stack {};
    uint8_t pc_stack_pointer { 0 };
    int8_t waiting_key { -1 };
    uint16_t width { 64 };
    uint16_t height { 32 };

    virtual void execute_instruction(uint8_t instruction, uint8_t nibble_x, uint8_t nibble_y, uint8_t nibble_n, uint8_t nibble_nn, uint16_t nibble_nnn);
    virtual void math_instructions(uint8_t n, uint8_t x, uint8_t y);
    virtual void f_instructions(uint8_t nn, uint8_t x, uint8_t y);
    virtual void draw(uint16_t x, uint16_t y, uint16_t h);
};
