#include <raylib.h>
#include <format>

#include "ui.h"

namespace UI {
    void draw(App& app, const uint64_t &hz) {
        DrawFPS(8, 6);
        std::string opcode = std::format("0x{:02X}", app.emu.get_opcode());
        DrawText(opcode.c_str(), 8, 30, 18, GREEN);
        DrawText(std::to_string(hz).c_str(), 8, 48, 18, GREEN);
    }
}
