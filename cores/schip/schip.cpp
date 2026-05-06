#include "schip.h"

void SChip::init() {
    Chip8::init();
    for (int i = 0; i < SChip::SUPER_FONT_DATA.size(); i++) {
        const int ram_idx {i + 0xA0};
        ram[ram_idx] = SChip::SUPER_FONT_DATA[i];
    }
}

void SChip::execute_instruction(uint8_t instruction, uint8_t x, uint8_t y, uint8_t n, uint8_t nn, uint16_t nnn) {
    switch (instruction) {
        case 0x0: {
            if ((nn & 0xF0) == 0xC0) {
                for (int dy = height - 1; dy >= n; dy--) {
                    for (int dx = 0; dx < width - 1; dx++) {
                        int index {(dy * width) + dx};
                        int next_index {((dy - n) * width) + dx};
                        vram[index] = vram[next_index];
                    }
                }
                for (int dy = 0; dy <= n -1; dy++) {
                    for (int dx = 0; dx < width /*- 1*/; dx++) {
                        int index {(dy * width) + dx};
                        vram[index] = false;
                    }
                }
                return;
            } else if (nn == 0xFB) {
                for (int dy = 0; dy < height; dy++) {
                    for (int dx = width - 1; dx >= 4; dx--) {
                        int index {(dy * width) + dx};
                        int next_index {(dy * width) + (dx - 4)};
                        vram[index] = vram[next_index];
                        vram[next_index] = false;
                    }
                }
                return;
            } else if (nn == 0xFC) {
                for (int dy = 0; dy < height; dy++) {
                    for (int dx = 0; dx < width - 5; dx++) {
                        int index {(dy * width) + dx};
                        int next_index {(dy * width) + (dx + 4)};
                        vram[index] = vram[next_index];
                        vram[next_index] = false;
                    }
                }
                return;
            } else if (nn == 0xFD) {
                Chip8::init();
                return;
            } else if (nn == 0xFE) {
                if (quirks.emulate_clear_on_mode_switch) {
                    vram.fill(false);
                }
                width = 64;
                height = 32;
                return;
            } else if (nn == 0xFF) {
                if (quirks.emulate_clear_on_mode_switch) {
                    vram.fill(false);
                }
                width = 128;
                height = 64;
                return;
            }
            break;
        }
        case 0xF:
            if (nn == 0x30) {
                i_register = 0xA0 + (10 * v_registers[x]);
                return;
            } else if (nn == 0x75) {
                uint8_t limit = std::min(static_cast<int>(x), 7);
                for (int i = 0; i <= limit; i++) {
                    rpl[i] = v_registers[i];
                }
                return;
            } else if (nn == 0x85) {
                uint8_t limit = std::min(static_cast<int>(x), 7);
                for (int i = 0; i <= limit; i++) {
                    v_registers[i] = rpl[i];
                }
                return;
            }
            break;
        default: {
            break;
        }
    }

    Chip8::execute_instruction(instruction, x, y, n, nn, nnn);
}

void SChip::draw(uint16_t x, uint16_t y, uint16_t h) {
    if (h != 0) {
        Chip8::draw(x, y, h);
        return;
    }
    x = x % width;
    y = y % height;
    h = 16;
    v_registers[0xF] = 0;

    for (int row = 0; row < h; row++) {
        const uint16_t sprite {static_cast<uint16_t>( (ram[i_register + row * 2] << 8) | ram[(i_register + row * 2) + 1] )};
        for (int i = 0; i < 16; i++) {
            const uint16_t cord_x = x + i;
            const uint16_t cord_y = y + row;
            if (cord_x >= width or cord_y >= height) {
                continue;
            }
            const bool is_sprite_pixel_on { ( (sprite >> (15 - i) ) & 1 ) != 0 };
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
            if (quirks.emulate_vblank_wait) {
                vblank = false;
            }

        }
    }
}