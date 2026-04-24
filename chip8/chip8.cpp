#include "chip8.h"
#include <fstream>
#include <random>

void init(Chip8& chip8) {
    chip8.ram.fill(0x00);
    chip8.vram.fill(false);
    chip8.v_registers.fill(0x00);
    chip8.input.fill(false);
    for (int i = 0; i < chip8.FONT_DATA.size(); i++) {
        const int ram_idx {i + 0x50};
        chip8.ram[ram_idx] = chip8.FONT_DATA[i];
    }
}

bool load_rom(Chip8& chip8, const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    if (size > (4096 - 0x200)) {
        return false;
    }
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&chip8.ram[0x200]), size);
    return true;
}

void run(Chip8& chip8)
{
    const uint16_t opcode {static_cast<uint16_t>( (chip8.ram[chip8.pc] << 8) | (chip8.ram[chip8.pc + 1]) )};
    const uint8_t instruction {static_cast<uint8_t>( (opcode & 0xF000) >> 12 )};
    const uint8_t nibble_x {static_cast<uint8_t>( (opcode & 0x0F00) >> 8 )};
    const uint8_t nibble_y {static_cast<uint8_t>( (opcode & 0x00F0) >> 4 )};
    const uint8_t nibble_n {static_cast<uint8_t>( opcode & 0x000F )};
    const uint8_t nibble_nn {static_cast<uint8_t>( opcode & 0x00FF )};
    const uint16_t nibble_nnn {static_cast<uint16_t>( opcode & 0x0FFF )};
    chip8.pc += 2;

    switch (instruction) {
        case 0x0: {
            if (nibble_nn == 0x00E0) {
                chip8.vram.fill(false);
            } else if (nibble_nn == 0x00EE) {
                chip8.pc = chip8.pc_stack.top();
                chip8.pc_stack.pop();
            }
            break;
        }
        case 0x1: {
            chip8.pc = nibble_nnn;
            break;
        }
        case 0x2: {
            chip8.pc_stack.push(chip8.pc);
            chip8.pc = nibble_nnn;
            break;
        }
        case 0x3: {
            if (chip8.v_registers[nibble_x] == nibble_nn) {
                chip8.pc += 2;
            }
            break;
        }
        case 0x4: {
            if (chip8.v_registers[nibble_x] != nibble_nn) {
                chip8.pc += 2;
            }
            break;
        }
        case 0x5: {
            if (chip8.v_registers[nibble_x] == chip8.v_registers[nibble_y]) {
                chip8.pc += 2;
            }
            break;
        }
        case 0x6: {
            chip8.v_registers[nibble_x] = nibble_nn;
            break;
        }
        case 0x7: {
            chip8.v_registers[nibble_x] = chip8.v_registers[nibble_x] + nibble_nn;
            break;
        }
        case 0x8: {
            math_instructions(chip8, nibble_n, nibble_x, nibble_y);
            break;
        }
        case 0x9: {
            if (chip8.v_registers[nibble_x] != chip8.v_registers[nibble_y]) {
                chip8.pc += 2;
            }
            break;
        }
        case 0xA: {
            chip8.i_register = nibble_nnn;
            break;
        }
        case 0xB: {
            uint16_t address {};
            if (chip8.quirks.emulate_buggy_jump_offset) {
                address = nibble_nnn + chip8.v_registers[nibble_x];
            } else {
                address = nibble_nnn + chip8.v_registers[0];
            }
            chip8.pc = address;
            break;
        }
        case 0xC: {
            chip8.v_registers[nibble_x] = (rand() % 256) & nibble_nn;
            break;
        }
        case 0xD: {
            draw(chip8, chip8.v_registers[nibble_x], chip8.v_registers[nibble_y], nibble_n);
            break;
        }
        case 0xE: {
            if (nibble_nn == 0x9E) {
                if (chip8.input[chip8.v_registers[nibble_x]] == true) {
                    chip8.pc += 2;
                }
            } else if (nibble_nn == 0xA1) {
                if (chip8.input[chip8.v_registers[nibble_x]] == false) {
                    chip8.pc += 2;
                }
            }
            break;
        }
        case 0xF: {
            f_instructions(chip8, nibble_nn, nibble_x, nibble_y);
            break;
        }
        default: {
            break;
        }
    }
};

void update_timers(Chip8& chip8) {
    if (chip8.delay_timer > 0) {
        chip8.delay_timer -= 1;
    };
    if (chip8.sound_timer > 0) {
        chip8.sound_timer -= 1;
    };
}

void math_instructions(Chip8& chip8, uint8_t n, uint8_t x, uint8_t y) {
    const uint8_t register_x {chip8.v_registers[x]};
    const uint8_t register_y {chip8.v_registers[y]};
    switch (n) {
        case 0x0: {
            chip8.v_registers[x] = register_y;
            break;
        }
        case 0x1: {
            chip8.v_registers[x] = register_x | register_y;
            if (chip8.quirks.emulate_vf_reset) {
                chip8.v_registers[0xF] = 0;
            }
            break;
        }
        case 0x2: {
            chip8.v_registers[x] = register_x & register_y;
            if (chip8.quirks.emulate_vf_reset) {
                chip8.v_registers[0xF] = 0;
            }
            break;
        }
        case 0x3: {
            chip8.v_registers[x] = register_x ^ register_y;
            if (chip8.quirks.emulate_vf_reset) {
                chip8.v_registers[0xF] = 0;
            }
            break;
        }
        case 0x4: {
            const uint16_t sum {static_cast<uint16_t>( register_x + register_y )};
            chip8.v_registers[x] = sum;
            if (sum > 255) {
                chip8.v_registers[0xF] = 1;
            } else {
                chip8.v_registers[0xF] = 0;
            }
            break;
        }
        case 0x5: {
            chip8.v_registers[x] = register_x - register_y;
            if (register_x >= register_y) {
                chip8.v_registers[0xF] = 1;
            } else {
                chip8.v_registers[0xF] = 0;
            }
            break;
        }
        case 0x6: {
            if (chip8.quirks.emulate_old_shift) {
                chip8.v_registers[x] = register_y >> 1;
            } else {
                chip8.v_registers[x] = register_x >> 1;
            chip8.v_registers[0xF] = register_x & 0x1;
            }
            break;
        }
        case 0x7: {
            if (register_x > register_y) {
                chip8.v_registers[0xF] = 0;
            } else {
                chip8.v_registers[0xF] = 1;
            }
            break;
        }
        case 0xE: {
            chip8.v_registers[0xF] = (register_x & 0x80) >> 7;
            if (chip8.quirks.emulate_old_shift) {
                chip8.v_registers[x] = register_y << 1;
            } else {
                chip8.v_registers[x] = register_x << 1;
            }
            break;
        }
        default: {
            break;
        }
    }
}

void f_instructions(Chip8& chip8, uint8_t nn, uint8_t x, uint8_t y) {
    switch (nn) {
        case 0x0A: {
            chip8.pc -= 2;
            if (chip8.waiting_key == -1) {
                for (int i = 0; i < chip8.input.size(); i++) {
                    if (chip8.input[i] == true) {
                        chip8.waiting_key = i;
                    }
                }
            } else {
                if (!chip8.input[chip8.waiting_key]) {
                    chip8.v_registers[x] = chip8.waiting_key;
                    chip8.waiting_key = -1;
                    chip8.pc += 2;
                }
            }
            break;
        }
        case 0x07: {
            chip8.v_registers[x] = chip8.delay_timer;
            break;
        }
        case 0x15: {
            chip8.delay_timer = chip8.v_registers[x];
            break;
        }
        case 0x18: {
            chip8.sound_timer = chip8.v_registers[x];
            break;
        }
        case 0x1E: {
            chip8.i_register += chip8.v_registers[x];
            break;
        }
        case 0x29: {
            chip8.i_register = 0x50 + (5 * chip8.v_registers[x]);
            break;
        }
        case 0x33: {
            chip8.ram[chip8.i_register] = chip8.v_registers[x] / 100;
            chip8.ram[chip8.i_register + 1] = (chip8.v_registers[x] / 10) % 10;
            chip8.ram[chip8.i_register + 2] = chip8.v_registers[x] % 10;
            break;
        }
        case 0x55: {
            for (int i = 0; i < x + 1; i++) {
                chip8.ram[chip8.i_register + i] = chip8.v_registers[i];
            }
            if (chip8.quirks.emulate_old_load_store) {
                chip8.i_register += x + 1;
            }
            break;
        }
        case 0x65: {
            for (int i = 0; i < x + 1; i++) {
                chip8.v_registers[i] = chip8.ram[chip8.i_register + i];
            }
            if (chip8.quirks.emulate_old_load_store) {
                chip8.i_register += x + 1;
            }
            break;
        }
        default: {
            break;
        }
    }
}

void draw(Chip8& chip8, uint16_t x, uint16_t y, uint16_t height) {
    if (chip8.quirks.emulate_vblank_wait) {
        if (not chip8.vblank) {
            chip8.pc -= 2;
            return;
        }
    }
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
                chip8.vram[cord_y * 64 + cord_x] = false;
            } else {
                chip8.vram[cord_y * 64 + cord_x] = true;
            }
            if (chip8.quirks.emulate_vblank_wait) {
                chip8.vblank = false;
            }

        }
    }
}