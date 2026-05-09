#if defined(_WIN32)
#include "external/fix_win32_compatibility.h"
#endif
#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <format>
#include <portable-file-dialogs.h>

#include "ui.h"
#include "config.h"

namespace UI {
    Menu current_dropdown = Menu::NONE;
    Menu current_popup = Menu::NONE;
    
    static constexpr std::array<DropdownButton, 4> top_menu = {{
        { Menu::FILE,      "#1#File",      72.0f },
        { Menu::EMULATION, "#162#Emulation", 72.0f },
        { Menu::SETTINGS,  "#141#Settings",  72.0f },
        { Menu::ABOUT,     "#193#About",     72.0f },
    }};
    
    void draw(App& app, const uint64_t hz) {
        constexpr float top_panel_pos = 0;
        const float bottom_panel_pos = static_cast<float>( window_height ) - UI_PADDING;
        auto top_panel = Rectangle(0, top_panel_pos, static_cast<float>( window_width) , UI_PADDING);
        auto bottom_panel = Rectangle(0, bottom_panel_pos, static_cast<float>( window_width ), UI_PADDING);
        Color text_color { GetColor(GuiGetStyle( TEXT, TEXT_COLOR_NORMAL )) };
        const std::string opcode = std::format("0x{:02X}", app.emu.get_opcode());
        const char* opcode_c = opcode.c_str();

        GuiDummyRec(top_panel, "");
        GuiDummyRec(bottom_panel, "");
        GuiDrawText(opcode_c, bottom_panel, TEXT_ALIGN_RIGHT, text_color);
        GuiDrawText((std::string("hz: ") + std::to_string(hz)).c_str(), bottom_panel, TEXT_ALIGN_LEFT, text_color);
        GuiDrawText(app.rom_name.c_str(), bottom_panel, TEXT_ALIGN_MIDDLE, text_color);

        float active_dropdown_x = 0.0f;
        float current_x = 0.0f;
        for (const DropdownButton& item : top_menu ) {
            Rectangle btn_rect = { current_x, 0.0f, item.width, UI_PADDING };

            if (GuiButton(btn_rect, item.text)) {
                if (current_dropdown == item.id) {
                    current_dropdown = Menu::NONE;
                } else {
                    current_dropdown = item.id;
                }
            }
            if (current_dropdown == item.id) {
                active_dropdown_x = current_x;
            }
            current_x += item.width + 4;
        }

        switch (current_dropdown) {
            case Menu::FILE: {
                if (GuiButton(Rectangle(active_dropdown_x, 16.0f, 68.0f, UI_PADDING), "Open ROM")) {
                    const std::vector<std::string> selected_rom { pfd::open_file("Select a .ch8 Rom file", ".", {
                        ".ch8", "*.ch8",
                        "All Files", "*",
                        }
                    ).result() };
                    if (!selected_rom.empty()) {
                        app.emu.init();
                        app.rom_path = selected_rom[0];
                        app.rom_name = selected_rom[0].substr(selected_rom[0].find_last_of("/\\") + 1);
                        app.emu.load_rom(app.rom_path);
                        app.ready = true;
                        current_dropdown = Menu::NONE;
                    } else {
                        current_dropdown = Menu::NONE;
                    }
                }
                if (!app.ready) {
                    GuiDisable();
                }
                if (GuiButton(Rectangle(active_dropdown_x, 32.0f, 68.0f, UI_PADDING), "Close ROM")) {
                    app.ready = false;
                    app.rom_name = "None";
                    app.emu.init();
                    current_dropdown = Menu::NONE;
                }
                GuiEnable();
                break;
            }
            case Menu::EMULATION: {
                if (!app.ready) {
                    GuiDisable();
                }
                GuiToggle(Rectangle(active_dropdown_x, 16.0f, 68.0f, UI_PADDING), "Pause", &app.paused);
                if (GuiButton(Rectangle(active_dropdown_x, 32.0f, 68.0f, UI_PADDING), "Reset")) {
                    app.ready = false;
                    app.emu.init();
                    app.emu.load_rom(app.rom_path);
                    app.ready = true;
                    current_dropdown = Menu::NONE;
                }
                GuiEnable();
                GuiDisable();
                if (GuiButton(Rectangle(active_dropdown_x, 48.0f, 68.0f, UI_PADDING), "Save State")) {

                }
                if (GuiButton(Rectangle(active_dropdown_x, 64.0f, 68.0f, UI_PADDING), "Load State")) {

                }
                GuiEnable();
                break;
            }
            case Menu::SETTINGS: {
                current_popup = Menu::SETTINGS;
                break;
            }
            case Menu::ABOUT: {
                current_popup = Menu::ABOUT;
                break;
            }
            default: {
                break;
            }
        }
    }
}
