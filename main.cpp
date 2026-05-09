#include <raylib.h>
#include <cmath>
#include <fstream>
#include <map>

#include "ui/ui.h"
#include "app.h"
#include "config.h"

constexpr double TIMER_RATE = 1.0 / 60.0;
static const std::map<int, uint8_t> KEY_MAP = {
    {KEY_ONE, 0x1}, {KEY_TWO, 0x2}, {KEY_THREE, 0x3}, {KEY_FOUR, 0xC},
    {KEY_Q,   0x4}, {KEY_W,   0x5}, {KEY_E,     0x6}, {KEY_R,    0xD},
    {KEY_A,   0x7}, {KEY_S,   0x8}, {KEY_D,     0x9}, {KEY_F,    0xE},
    {KEY_Z,   0xA}, {KEY_X,   0x0}, {KEY_C,     0xB}, {KEY_V,    0xF}
};

#define AUDIO_BUFFER_SIZE 4096

void check_input(Chip8& emu) {
    for (const auto& [key, chip8_key] : KEY_MAP ) {
        emu.input[chip8_key] = IsKeyDown(key);
    }
}

int main()
{
    int last_display_hz = -1;
    InitWindow(window_width, window_height, "Hello Raylib");
    SetWindowState(FLAG_VSYNC_HINT);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    double timer_accumulator = 0.0;
    InitAudioDevice();
    App app;
    app.init();

    SetAudioStreamBufferSizeDefault(AUDIO_BUFFER_SIZE);
    float buffer[AUDIO_BUFFER_SIZE] = {};
    int sineIndex = 0;
    PlayAudioStream(app.audio_stream);

    app.emu.init();


    uint16_t current_width = app.emu.get_width();
    uint16_t current_height = app.emu.get_height();

    while (!WindowShouldClose())
    {
        const double frame_start_time = GetTime();
        const int current_monitor = GetCurrentMonitor();
        const int display_hz = std::max(1, GetMonitorRefreshRate(current_monitor));
        timer_accumulator += GetFrameTime();
        window_width = GetScreenWidth();
        window_height = GetScreenHeight();

        if (display_hz != last_display_hz) {
            SetTargetFPS(display_hz);
            last_display_hz = display_hz;
        }

        if (app.emu.get_width() != current_width || app.emu.get_height() != current_height) {
            UnloadRenderTexture(app.screen_tex);
            current_width = app.emu.get_width();
            current_height = app.emu.get_height();
            app.screen_tex = LoadRenderTexture(current_width, current_height);
        }

        float scale_x = static_cast<float>(window_width) / static_cast<float>(current_width);
        float scale_y = static_cast<float>(window_height - (UI_PADDING * 2) ) / static_cast<float>(current_height);
        float scale = std::min(scale_x, scale_y);

        float viewport_width = static_cast<float>(current_width) * scale;
        float viewport_height = static_cast<float>(current_height) * scale;

        const Rectangle renderTextureDest = {
            static_cast<float>(window_width - viewport_width)/ 2.0f,
            static_cast<float>(window_height - viewport_height) / 2.0f,
            viewport_width, viewport_height
        };
        const Rectangle renderTextureSrc {
            0.0f,
            0.0f,
            static_cast<float>(current_width),
            static_cast<float>(-current_height)
        };
        constexpr Vector2 renderTextureOrig = {0};

        uint16_t instructions_per_frame { static_cast<uint16_t>( app.hz / display_hz ) };
        uint64_t instructions_this_frame { 0 };

        if (app.ready and !app.paused) {
            check_input(app.emu);
            if (timer_accumulator >= TIMER_RATE) {
                app.emu.update_timers();
                timer_accumulator = 0.0;
            }
            app.emu.vblank = true;
            if (not app.unlocked_speed) {
                for (int i = 0; i < instructions_per_frame; i++)
                {
                    app.emu.run();
                    instructions_this_frame++;
                };
            } else {
                while ( (GetTime() - frame_start_time) < (1.0 / display_hz) ) {
                    app.emu.run();
                    instructions_this_frame++;
                }
            }
            if (app.emu.sound_timer != 0) {
                if (!IsAudioStreamPlaying(app.audio_stream))
                    ResumeAudioStream(app.audio_stream);

                if (IsAudioStreamProcessed(app.audio_stream)) { // ty raylib raw audio example
                    for (float & i : buffer) {
                        int wavelength = 44100/app.emu.audio_frequency;
                        i = std::sin(2.0*PI*sineIndex/wavelength)>=0.0 ? 1.0:-1.0; // thanks google
                        sineIndex++;
                        if (sineIndex >= wavelength)
                        {
                            sineIndex = 0;
                        }
                    }
                    UpdateAudioStream(app.audio_stream, buffer, AUDIO_BUFFER_SIZE);
                }
            } else {
                PauseAudioStream(app.audio_stream);
            }
        } else {
            PauseAudioStream(app.audio_stream);
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
            const uint64_t hz_per_second { instructions_this_frame * display_hz};
            DrawTexturePro(app.screen_tex.texture,
                renderTextureSrc,
                renderTextureDest,
                renderTextureOrig,
                0,
                WHITE);
            UI::draw(app, hz_per_second);

        EndDrawing();


    };
    app.cleanup();
    CloseWindow();
    return 0;
}