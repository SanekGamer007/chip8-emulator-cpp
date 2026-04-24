#include "chip8.h"
#include <iostream>

void init(Chip8& chip8) {
    for (int i = 0; i < chip8.FONT_DATA.size(); i++) {
        const int ram_idx {i + 0x50};
        chip8.ram[ram_idx] = chip8.FONT_DATA[i];
    }
    chip8.ram[0x200] = 0x60;
    chip8.ram[0x201] = 0x01;
    chip8.ram[0x202] = 0x70;
    chip8.ram[0x203] = 0x01;
    chip8.ram[0x204] = 0x12;
    chip8.ram[0x205] = 0x02;
}

void run(Chip8& chip8) // Placeholder
{
    chip8.ram[0x200] = {0x00};
    chip8.ram[0x201] = {0xE0};
    const uint16_t opcode {static_cast<uint16_t>( (chip8.ram[chip8.pc] << 8) | (chip8.ram[chip8.pc + 1]) )};
    const uint8_t instruction {static_cast<uint8_t>( (opcode & 0xF000) >> 12 )};
    const uint8_t nibble_x {static_cast<uint8_t>( (opcode & 0x0F00) >> 8 )};
    const uint8_t nibble_y {static_cast<uint8_t>( (opcode & 0x00F0) >> 4 )};
    const uint8_t nibble_n {static_cast<uint8_t>( opcode & 0x000F )};
    const uint8_t nibble_nn {static_cast<uint8_t>( opcode & 0x00FF )};
    const uint16_t nibble_nnn {static_cast<uint16_t>( opcode & 0x0FFF )};
    std::cout << "0x" << std::uppercase << std::hex << opcode << '\n';
    chip8.pc += 2;

    switch (instruction) {
        case 0x0:
            if (nibble_nn == 0x00E0) {
                chip8.vram.fill(false);
            };
            break;
        case 0x1:
            chip8.pc = nibble_nnn;
            break;
        case 0x6:
            chip8.v_registers[nibble_x] = nibble_nn;
            break;
        case 0x7:
            chip8.v_registers[nibble_x] += nibble_nn;
        case 0xA:
            chip8.i_register = nibble_nnn;
        case 0xD:
            draw(chip8, nibble_x, nibble_y, nibble_n);
        default:
            break;
    }
};

void update_timers(Chip8& chip8) {
    if (chip8.delay_timer > 0) {
        chip8.delay_timer += 1;
    };
    if (chip8.sound_timer > 0) {
        chip8.sound_timer += 1;
    };
}

void draw(Chip8& chip8, uint16_t x, uint16_t y, uint16_t height) {
    x = x % 64;
    y = y % 32;
    chip8.v_registers[0xF] = 0;
    for (int row = 0; row < height; row++) {
        const uint8_t byte { chip8.ram[chip8.i_register + row] };
        for (int i = 0; i < 8; i++) {
            const uint16_t cord_x = x + i;
            const uint16_t cord_y = y + row;
            if (cord_x >= 64 or cord_y >= 32) {
                continue;
            }
            const bool is_sprite_pixel_on { ( (byte >> (7 - i) ) & 1 ) != 0 };
            const bool is_vram_pixel_on { chip8.vram[cord_y * 64 + cord_x] };
            if (not is_sprite_pixel_on) {
                continue;
            }
            if (is_vram_pixel_on) {
                chip8.v_registers[0xF] = 1;
                chip8.vram[cord_y * 64 + cord_x] = true;
            } else {
                chip8.vram[cord_y * 64 + cord_x] = false;
            }
        }
    }
}