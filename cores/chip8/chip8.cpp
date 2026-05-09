#include "chip8.h"
#include <fstream>
#include <random>
// TODO: Implement 'random' random
void Chip8::init() {
    ram.fill(0x00);
    vram.fill(false);
    v_registers.fill(0x00);
    i_register = 0x0;
    delay_timer = 0;
    sound_timer = 0;
    waiting_key = -1;
    width = 64;
    height = 32;
    pc = 0x200;
    input.fill(false);
    pc_stack = {};
    for (int i = 0; i < FONT_DATA.size(); i++) {
        const int ram_idx {i + 0x50};
        ram[ram_idx] = FONT_DATA[i];
    }
}

bool Chip8::load_rom(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    const std::streamsize size = file.tellg();
    if (size > (4096 - 0x200)) {
        return false;
    }
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&ram[0x200]), size);
    return true;
}

void Chip8::run()
{
    const uint16_t opcode = get_opcode();
    const uint8_t instruction {static_cast<uint8_t>( (opcode & 0xF000) >> 12 )};
    const uint8_t nibble_x {static_cast<uint8_t>( (opcode & 0x0F00) >> 8 )};
    const uint8_t nibble_y {static_cast<uint8_t>( (opcode & 0x00F0) >> 4 )};
    const uint8_t nibble_n {static_cast<uint8_t>( opcode & 0x000F )};
    const uint8_t nibble_nn {static_cast<uint8_t>( opcode & 0x00FF )};
    const uint16_t nibble_nnn {static_cast<uint16_t>( opcode & 0x0FFF )};
    pc += 2;
    execute_instruction(instruction, nibble_x, nibble_y, nibble_n, nibble_nn, nibble_nnn);
};

void Chip8::execute_instruction(uint8_t instruction, uint8_t x, uint8_t y, uint8_t n, uint8_t nn, uint16_t nnn) {
    switch (instruction) {
        case 0x0: {
            if (nn == 0xE0) {
                vram.fill(false);
            } else if (nn == 0xEE) {
                pc = pc_stack.top();
                pc_stack.pop();
            }
            break;
        }
        case 0x1: {
            pc = nnn;
            break;
        }
        case 0x2: {
            pc_stack.push(pc);
            pc = nnn;
            break;
        }
        case 0x3: {
            if (v_registers[x] == nn) {
                pc += 2;
            }
            break;
        }
        case 0x4: {
            if (v_registers[x] != nn) {
                pc += 2;
            }
            break;
        }
        case 0x5: {
            if (v_registers[x] == v_registers[y]) {
                pc += 2;
            }
            break;
        }
        case 0x6: {
            v_registers[x] = nn;
            break;
        }
        case 0x7: {
            v_registers[x] = v_registers[x] + nn;
            break;
        }
        case 0x8: {
            math_instructions(n, x, y);
            break;
        }
        case 0x9: {
            if (v_registers[x] != v_registers[y]) {
                pc += 2;
            }
            break;
        }
        case 0xA: {
            i_register = nnn;
            break;
        }
        case 0xB: {
            uint16_t address {};
            if (quirks.emulate_buggy_jump_offset) {
                address = nnn + v_registers[x];
            } else {
                address = nnn + v_registers[0];
            }
            pc = address;
            break;
        }
        case 0xC: {
            v_registers[x] = (rand() % 256) & nn;
            break;
        }
        case 0xD: {
            draw(v_registers[x], v_registers[y], n);
            break;
        }
        case 0xE: {
            if (nn == 0x9E) {
                if (input[v_registers[x]] == true) {
                    pc += 2;
                }
            } else if (nn == 0xA1) {
                if (input[v_registers[x]] == false) {
                    pc += 2;
                }
            }
            break;
        }
        case 0xF: {
            f_instructions(nn, x, y);
            break;
        }
        default: {
            break;
        }
    }
}

void Chip8::update_timers() {
    if (delay_timer > 0) {
        delay_timer -= 1;
    };
    if (sound_timer > 0) {
        sound_timer -= 1;
    };
}

uint16_t Chip8::get_opcode() const {
    return (ram[pc] << 8) | (ram[pc + 1]);
}

void Chip8::math_instructions(uint8_t n, uint8_t x, uint8_t y) {
    const uint8_t register_x {v_registers[x]};
    const uint8_t register_y {v_registers[y]};
    switch (n) {
        case 0x0: {
            v_registers[x] = register_y;
            break;
        }
        case 0x1: {
            v_registers[x] = register_x | register_y;
            if (quirks.emulate_vf_reset) {
                v_registers[0xF] = 0;
            }
            break;
        }
        case 0x2: {
            v_registers[x] = register_x & register_y;
            if (quirks.emulate_vf_reset) {
                v_registers[0xF] = 0;
            }
            break;
        }
        case 0x3: {
            v_registers[x] = register_x ^ register_y;
            if (quirks.emulate_vf_reset) {
                v_registers[0xF] = 0;
            }
            break;
        }
        case 0x4: {
            const uint16_t sum {static_cast<uint16_t>( register_x + register_y )};
            v_registers[x] = sum;
            if (sum > 255) {
                v_registers[0xF] = 1;
            } else {
                v_registers[0xF] = 0;
            }
            break;
        }
        case 0x5: {
            v_registers[x] = register_x - register_y;
            if (register_x >= register_y) {
                v_registers[0xF] = 1;
            } else {
                v_registers[0xF] = 0;
            }
            break;
        }
        case 0x6: {
            if (quirks.emulate_old_shift) {
                v_registers[x] = register_y >> 1;
            } else {
                v_registers[x] = register_x >> 1;
            }
            v_registers[0xF] = register_x & 0x1;
            break;
        }
        case 0x7: {
            v_registers[x] = register_y - register_x;
            if (register_x > register_y) {
                v_registers[0xF] = 0;
            } else {
                v_registers[0xF] = 1;
            }
            break;
        }
        case 0xE: {
            if (quirks.emulate_old_shift) {
                v_registers[x] = register_y << 1;
            } else {
                v_registers[x] = register_x << 1;
            }
            v_registers[0xF] = (register_x & 0x80) >> 7;
            break;
        }
        default: {
            break;
        }
    }
}

void Chip8::f_instructions(uint8_t nn, uint8_t x, uint8_t y) {
    switch (nn) {
        case 0x0A: {
            pc -= 2;
            if (waiting_key == -1) {
                for (int i = 0; i < input.size(); i++) {
                    if (input[i] == true) {
                        waiting_key = static_cast<int8_t>(i);
                    }
                }
            } else {
                if (!input[waiting_key]) {
                    v_registers[x] = waiting_key;
                    waiting_key = -1;
                    pc += 2;
                }
            }
            break;
        }
        case 0x07: {
            v_registers[x] = delay_timer;
            break;
        }
        case 0x15: {
            delay_timer = v_registers[x];
            break;
        }
        case 0x18: {
            sound_timer = v_registers[x];
            break;
        }
        case 0x1E: {
            i_register += v_registers[x];
            break;
        }
        case 0x29: {
            i_register = 0x50 + (5 * v_registers[x]);
            break;
        }
        case 0x33: {
            ram[i_register] = v_registers[x] / 100;
            ram[i_register + 1] = (v_registers[x] / 10) % 10;
            ram[i_register + 2] = v_registers[x] % 10;
            break;
        }
        case 0x55: {
            for (int i = 0; i < x + 1; i++) {
                ram[i_register + i] = v_registers[i];
            }
            if (quirks.emulate_old_load_store) {
                i_register += x + 1;
            }
            break;
        }
        case 0x65: {
            for (int i = 0; i < x + 1; i++) {
                v_registers[i] = ram[i_register + i];
            }
            if (quirks.emulate_old_load_store) {
                i_register += x + 1;
            }
            break;
        }
        default: {
            break;
        }
    }
}

void Chip8::draw(uint16_t x, uint16_t y, uint16_t h) {
    if (quirks.emulate_vblank_wait) {
        if (not vblank) {
            pc -= 2;
            return;
        }
    }
    x = x % width;
    y = y % height;
    v_registers[0xF] = 0;
    for (int row = 0; row < h; row++) {
        const uint8_t byte { ram[i_register + row] };
        for (int i = 0; i < 8; i++) {
            const uint16_t cord_x = x + i;
            const uint16_t cord_y = y + row;
            if (cord_x >= width or cord_y >= height) {
                continue;
            }
            const bool is_sprite_pixel_on { ( (byte >> (7 - i) ) & 1 ) != 0 };
            const bool is_vram_pixel_on { ( vram[cord_y * width + cord_x] != 0) };
            if (not is_sprite_pixel_on) {
                continue;
            }
            if (is_vram_pixel_on) {
                v_registers[0xF] = 1;
                vram[cord_y * width + cord_x] = false;
            } else {
                vram[cord_y * width + cord_x] = true;
            }
        }
    }
    if (quirks.emulate_vblank_wait) {
        vblank = false;
    }
}