#include <raylib.h>
#include "external/portable-file-dialogs/portable-file-dialogs.h"
#include <fstream>
#include <map>
#include <cmath>

#include <print>

#include "cores/chip8/chip8.h"
#include "cores/schip/schip.h"
#include "ui/ui.h"
#include "app.h"

constexpr int VIEWPORT_WIDTH = 512;
constexpr int VIEWPORT_HEIGHT = 256;

#define AUDIO_BUFFER_SIZE 4096

void check_input(Chip8& emu) {
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
    int screen_width = 768;
    int screen_height = 384;
    InitWindow(screen_width, screen_height, "Hello Raylib");
    SetTargetFPS(60);
    InitAudioDevice();
    App app;

    SetAudioStreamBufferSizeDefault(AUDIO_BUFFER_SIZE);
    float buffer[AUDIO_BUFFER_SIZE] = {};
    int sineIndex = 0;
    PlayAudioStream(app.audio_stream);

    app.emu.init();
    const std::vector<std::string> selected_rom { pfd::open_file("Select a .ch8 Rom file", ".", {
        ".ch8", "*.ch8",
        "All Files", "*",
        }
    ).result() };
    if (!selected_rom.empty()) {
        app.emu.load_rom(selected_rom[0]);
    } else {
        return 0;
    }

    uint16_t current_width = app.emu.get_width();
    uint16_t current_height = app.emu.get_height();

    while (!WindowShouldClose())
    {
        double frame_start_time = GetTime();

        if (app.emu.get_width() != current_width || app.emu.get_height() != current_height) {
            UnloadRenderTexture(app.screen_tex);
            current_width = app.emu.get_width();
            current_height = app.emu.get_height();
            app.screen_tex = LoadRenderTexture(current_width, current_height);
        }

        const Rectangle renderTextureDest = { (screen_width - VIEWPORT_WIDTH) / 2.0f, (screen_height - VIEWPORT_HEIGHT) / 2.0f, VIEWPORT_WIDTH, VIEWPORT_HEIGHT };
        const Rectangle renderTextureSrc {
            0.0f,
            0.0f,
            static_cast<float>(current_width),
            static_cast<float>(-current_height)
        };
        constexpr Vector2 renderTextureOrig = {0};

        uint16_t instructions_per_frame { static_cast<uint16_t>(app.hz / 60) };
        uint64_t instructions_this_frame { 0 };

        check_input(app.emu);
        app.emu.update_timers();
        app.emu.vblank = true;
        if (not app.unlocked_speed) {
            for (int i = 0; i < instructions_per_frame; i++)
            {
                app.emu.run();
                instructions_this_frame++;
            };
        } else {
            while (GetTime() - frame_start_time < (1.0 / 60.0)) {
                app.emu.run();
                instructions_this_frame++;
            }
        }

        if (app.emu.sound_timer != 0) {
            if (IsAudioStreamProcessed(app.audio_stream)) { // ty raylib raw audio example
                for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
                    int wavelength = 44100/440;
                    buffer[i] = sin(2*PI*sineIndex/wavelength)>=0.0 ? 1.0:-1.0; // thanks google
                    sineIndex++;
                    if (sineIndex >= wavelength)
                    {
                        sineIndex = 0;
                    }
                }
                UpdateAudioStream(app.audio_stream, buffer, AUDIO_BUFFER_SIZE);
            }
        }



        BeginTextureMode(app.screen_tex);
            ClearBackground(app.palette[0]);
            for (int y = 0; y < current_height; y++) {
                for (int x = 0; x < current_width; x++) {
                    if (app.emu.get_vram()[y * current_width + x]) {
                        DrawPixel(x, y, app.palette[1]);
                    };
                };
            };
        EndTextureMode();

        BeginDrawing();
            ClearBackground(app.palette[0]);
            const uint64_t hz_per_second { instructions_this_frame * 60};
            UI::draw(app, hz_per_second);
            DrawTexturePro(app.screen_tex.texture, renderTextureSrc, renderTextureDest, renderTextureOrig, 0, WHITE);

        EndDrawing();


    };
    app.cleanup();
    CloseWindow();
    return 0;
}