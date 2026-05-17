#pragma once

#include <memory>
#include <raylib.h>

#include "cores/chip8/chip8.h"
#include "cores/schip/schip.h"

struct App {
    std::unique_ptr<Chip8> emu { std::make_unique<Chip8>() };

    RenderTexture2D screen_tex {};
    AudioStream audio_stream {};

    int hz = 2000;
    bool unlocked_speed = false;
    bool ready = false;
    bool paused = false;

    Color palette[4] { BLACK, WHITE, WHITE, WHITE };
    std::string rom_path {};
    std::string rom_name = "None";

    void init() {
        screen_tex = LoadRenderTexture(emu->get_width(), emu->get_height());
        audio_stream = LoadAudioStream(44100, 32, 1);
    }

    void cleanup() const {
        UnloadRenderTexture(screen_tex);
        UnloadAudioStream(audio_stream);
    }
};
