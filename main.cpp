#include <raylib.h>
#include "portable-file-dialogs.h"

#include "chip8/chip8.h"

#define DEFAULT_V_SCREEN_WIDTH 64
#define DEFAULT_V_SCREEN_HEIGHT 32
#define SCREEN_WIDTH  512
#define SCREEN_HEIGHT 256

uint16_t hz { 60 };
Chip8 emu {};

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello Raylib");
    SetTargetFPS(60);

    const RenderTexture2D renderTexture = LoadRenderTexture(DEFAULT_V_SCREEN_WIDTH, DEFAULT_V_SCREEN_HEIGHT);
    const uint16_t instructions_per_second { static_cast<uint16_t>(hz / 60) };

    const std::vector<std::string> selected_rom { pfd::open_file("Select a .ch8 Rom file", ".", {
        ".ch8", "*.ch8",
        "All Files", "*",
        }
    ).result() };
    if (!selected_rom.empty())
        std::cout << "User selected file " << selected_rom[0] << "\n";
    init(emu);

    while (!WindowShouldClose())
    {
        constexpr Rectangle renderTextureDest = { 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT };
        constexpr Rectangle renderTextureSrc { 0.0f, 0.0f, DEFAULT_V_SCREEN_WIDTH, -DEFAULT_V_SCREEN_HEIGHT };
        constexpr Vector2 renderTextureOrig = {0};

        update_timers(emu);
        for (int i = 0; i < instructions_per_second; i++)
        {
            run(emu);
        };

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
            DrawFPS(8, 6);
        EndDrawing();
    };
    UnloadRenderTexture(renderTexture);
    CloseWindow();
    return 0;
}
