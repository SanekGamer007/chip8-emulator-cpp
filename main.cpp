#include <raylib.h>
#include <fstream>
#include "external/portable-file-dialogs/portable-file-dialogs.h"

#include "cores/chip8/chip8.h"
#include "cores/schip/schip.h"
#include "ui/ui.h"


#define VIEWPORT_WIDTH  512
#define VIEWPORT_HEIGHT 256

#define SCREEN_WIDTH  768
#define SCREEN_HEIGHT 384

bool unlocked_speed { false };

uint16_t hz { 20000 };
SChip emu {};

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

    RenderTexture2D renderTexture = LoadRenderTexture(emu.get_width(), emu.get_height());
    emu.init();
    const std::vector<std::string> selected_rom { pfd::open_file("Select a .ch8 Rom file", ".", {
        ".ch8", "*.ch8",
        "All Files", "*",
        }
    ).result() };
    if (!selected_rom.empty()) {
        emu.load_rom(selected_rom[0]);
    } else {
        return 0;
    }

    uint16_t current_width = emu.get_width();
    uint16_t current_height = emu.get_height();

    while (!WindowShouldClose())
    {
        if (emu.get_width() != current_width || emu.get_height() != current_height) {
            UnloadRenderTexture(renderTexture);
            current_width = emu.get_width();
            current_height = emu.get_height();
            renderTexture = LoadRenderTexture(emu.get_width(), emu.get_height());
        }

        uint16_t instructions_per_frame { static_cast<uint16_t>(hz / 60) };
        uint64_t instructions_this_frame { 0 };
        double frame_start_time = GetTime();
        constexpr Rectangle renderTextureDest = { (SCREEN_WIDTH - VIEWPORT_WIDTH) / 2.0f, (SCREEN_HEIGHT - VIEWPORT_HEIGHT) / 2.0f, VIEWPORT_WIDTH, VIEWPORT_HEIGHT };
        const Rectangle renderTextureSrc {
            0.0f,
            0.0f,
            static_cast<float>(emu.get_width()),
            static_cast<float>(-emu.get_height())
        };
        constexpr Vector2 renderTextureOrig = {0};

        check_input();
        emu.update_timers();
        emu.vblank = true;
        if (not unlocked_speed) {
            for (int i = 0; i < instructions_per_frame; i++)
            {
                emu.run();
                instructions_this_frame++;
            };
        } else {
            while (GetTime() - frame_start_time < (1.0 / 60.0)) {
                emu.run();
                instructions_this_frame++;
            }
        }

        BeginTextureMode(renderTexture);
            ClearBackground(BLACK);
            for (int y = 0; y < current_height; y++) {
                for (int x = 0; x < current_width; x++) {
                    if (emu.get_vram()[y * current_width + x]) {
                        DrawPixel(x, y, WHITE);
                    };
                };
            };
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            const uint64_t hz_per_second { instructions_this_frame * 60};
            UI::draw(emu, hz_per_second);
            DrawTexturePro(renderTexture.texture, renderTextureSrc, renderTextureDest, renderTextureOrig, 0, WHITE);

        EndDrawing();
    };
    UnloadRenderTexture(renderTexture);
    CloseWindow();
    return 0;
}

