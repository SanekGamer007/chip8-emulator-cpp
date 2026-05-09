#pragma once
#include "app.h"

namespace UI {
    enum class Menu {
        NONE,
        FILE,
        EMULATION,
        SETTINGS,
        ABOUT,
    };

    struct DropdownButton {
        Menu id;
        const char* text;
        float width;
    };

    void draw(App& app, uint64_t hz);
}
