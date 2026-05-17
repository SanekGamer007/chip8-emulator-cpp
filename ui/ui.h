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

    inline bool ui_focus = false;
    static constexpr const unsigned char LOGO[] = {
        #embed "../assets/logo.png" // why the fuck it's not able to use the project root, #include can, why YOU can't...
    };
    inline Image logo_img { };
    inline Texture2D logo_tex { }; // holy fuck all of this is so ugly.
    constexpr int UI_VERT_PADDING = 16;
    constexpr int UI_HORZ_PADDING = 8;
    constexpr int TOP_DROPDOWN_BTN_SIZE = 72;
    constexpr Vector2 SETTINGS_MENU_SIZE = Vector2(220.0f, 260.0f);
    constexpr Vector2 ABOUT_MENU_SIZE = Vector2(200.0f, 160.0f);
    constexpr float ROW_HEIGHT = 24.0f;

    void init(const App& app);
    void draw(App& app, uint64_t hz);
    void apply_core(App& app, int core);
    void apply_quirks(const App& app, int core);
}
