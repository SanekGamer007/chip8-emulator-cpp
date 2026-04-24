#include <raylib.h>
#include <fstream>
#include <format>
#include "portable-file-dialogs.h"

#include "chip8/chip8.h"

#define DEFAULT_V_SCREEN_WIDTH 64
#define DEFAULT_V_SCREEN_HEIGHT 32
#define SCREEN_WIDTH  512
#define SCREEN_HEIGHT 256

bool unlocked_speed { false };

uint16_t hz { 2000 };
Chip8 emu {};

void check_input() {
    emu.input[0x1] = IsKeyDown(KEY_ONE);
    emu.input[0x2] = IsKeyDown(KEY_TWO);
    emu.input[0x3] = IsKeyDown(KEY_THREE);
    emu.input[0xC] = IsKeyDown(KEY_FOUR);
    emu.input[0x4] = IsKeyDown(KEY_Q);
    emu.input[0x5] = IsKeyDown(KEY_W);
    emu.input[0x6] = IsKeyDown(KEY_E);
    emu.input[0xD] = IsKeyDown(KEY_R);
    emu.input[0x7] = IsKeyDown(KEY_A);
    emu.input[0x8] = IsKeyDown(KEY_S);
    emu.input[0x9] = IsKeyDown(KEY_D);
    emu.input[0xE] = IsKeyDown(KEY_F);
    emu.input[0xA] = IsKeyDown(KEY_Z);
    emu.input[0x0] = IsKeyDown(KEY_X);
    emu.input[0xB] = IsKeyDown(KEY_C);
    emu.input[0xF] = IsKeyDown(KEY_V);
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello Raylib");
    SetTargetFPS(60);

    const RenderTexture2D renderTexture = LoadRenderTexture(DEFAULT_V_SCREEN_WIDTH, DEFAULT_V_SCREEN_HEIGHT);

    init(emu);
    const std::vector<std::string> selected_rom { pfd::open_file("Select a .ch8 Rom file", ".", {
        ".ch8", "*.ch8",
        "All Files", "*",
        }
    ).result() };
    if (!selected_rom.empty()) {
        load_rom(emu, selected_rom[0]);
    } else {
        return 0;
    }


    while (!WindowShouldClose())
    {
        uint16_t instructions_per_frame { static_cast<uint16_t>(hz / 60) };
        uint64_t instructions_this_frame { 0 };
        double frame_start_time = GetTime();

        constexpr Rectangle renderTextureDest = { 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT };
        constexpr Rectangle renderTextureSrc { 0.0f, 0.0f, DEFAULT_V_SCREEN_WIDTH, -DEFAULT_V_SCREEN_HEIGHT };
        constexpr Vector2 renderTextureOrig = {0};

        check_input();
        update_timers(emu);
        emu.vblank = true;
        if (not unlocked_speed) {
            for (int i = 0; i < instructions_per_frame; i++)
            {
                run(emu);
                instructions_this_frame++;
            };
        } else {
            while (GetTime() - frame_start_time < (1.0 / 60.0)) {
                run(emu);
                instructions_this_frame++;
            }
        }

        BeginTextureMode(renderTexture);
            ClearBackground(BLACK);
            for (int y = 0; y < 32; y++) {
                for (int x = 0; x < 64; x++) {
                    if (emu.vram[y * 64 + x]) {
                        DrawPixel(x, y, WHITE);
                    };
                };
            };
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(renderTexture.texture, renderTextureSrc, renderTextureDest, renderTextureOrig, 0, WHITE);
            DrawRectangle(0,0,96,72,Color(0,0,0,120));
            DrawFPS(8, 6);
            std::string opcode = std::format("0x{:02X}", ((emu.ram[emu.pc] << 8) | (emu.ram[emu.pc + 1])));
            DrawText(opcode.c_str(), 8, 26, 18, GREEN);
            std::string hz_per_sec = std::to_string(instructions_this_frame * 60);
            DrawText(hz_per_sec.c_str(), 8, 40, 18, GREEN);

        EndDrawing();
    };
    UnloadRenderTexture(renderTexture);
    CloseWindow();
    return 0;
}

