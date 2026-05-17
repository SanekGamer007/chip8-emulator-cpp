#if defined(_WIN32)
    #include "external/fix_win32_compatibility.h"
#endif
#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <format>
#include <ranges>
#include <portable-file-dialogs.h>

#include "ui.h"
#include "config.h"

namespace UI {
    Menu current_dropdown = Menu::NONE;
    Menu current_popup = Menu::NONE;

    static bool hz_edit_mode = false;
    static bool fps_edit_mode = false;
    static bool core_dropdown_edit_mode = false;
    int selected_core = 0;
    int current_core = 0;
    int selected_color_picker = 0;
    int temp_max_fps = max_fps;
    
    static constexpr std::array<DropdownButton, 4> top_menu = {{
        { Menu::FILE,      "#1#File",      72.0f },
        { Menu::EMULATION, "#162#Emulation", 72.0f },
        { Menu::SETTINGS,  "#141#Settings",  72.0f },
        { Menu::ABOUT,     "#193#About",     72.0f },
    }};

    void init(const App& app) {
        logo_img = LoadImageFromMemory(".png", UI::LOGO, sizeof(UI::LOGO));
        logo_tex = LoadTextureFromImage(logo_img);
        temp_max_fps = max_fps;
        apply_quirks(app, 0);
    }

    void draw(App& app, const uint64_t hz) {
        constexpr float top_panel_pos = 0;
        const float bottom_panel_pos = static_cast<float>( window_height ) - UI_VERT_PADDING;
        auto top_panel = Rectangle(0, top_panel_pos, static_cast<float>( window_width) , UI_VERT_PADDING);
        auto bottom_panel = Rectangle(0, bottom_panel_pos, static_cast<float>( window_width ), UI_VERT_PADDING);
        Color text_color { GetColor(GuiGetStyle( TEXT, TEXT_COLOR_NORMAL )) };
        const std::string opcode = std::format("0x{:02X}", app.emu->get_opcode());
        const char* opcode_c = opcode.c_str();

        GuiDummyRec(top_panel, "");
        GuiDummyRec(bottom_panel, "");
        GuiDrawText(opcode_c, bottom_panel, TEXT_ALIGN_RIGHT, text_color);
        GuiDrawText((std::string("hz: ") + std::to_string(hz)).c_str(), bottom_panel, TEXT_ALIGN_LEFT, text_color);
        GuiDrawText(app.rom_name.c_str(), bottom_panel, TEXT_ALIGN_MIDDLE, text_color);

        float active_dropdown_x = 0.0f;
        float current_x = 0.0f;
        for (const DropdownButton& item : top_menu ) {
            Rectangle btn_rect = { current_x, 0.0f, item.width, UI_VERT_PADDING };

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
            current_x += item.width + UI_HORZ_PADDING;
        }

        switch (current_dropdown) {
            case Menu::NONE:
                break;

            case Menu::FILE: {
                if (GuiButton(Rectangle(active_dropdown_x, 16.0f, TOP_DROPDOWN_BTN_SIZE, UI_VERT_PADDING), "Open ROM")) {
                    const std::vector<std::string> selected_rom { pfd::open_file("Select a .ch8 Rom file", ".", {
                        ".ch8", "*.ch8",
                        "All Files", "*",
                        }
                    ).result() };
                    if (!selected_rom.empty()) {
                        app.emu->init();
                        app.rom_path = selected_rom[0];
                        app.rom_name = selected_rom[0].substr(selected_rom[0].find_last_of("/\\") + 1);
                        app.emu->load_rom(app.rom_path);
                        app.ready = true;
                        current_dropdown = Menu::NONE;
                    } else {
                        current_dropdown = Menu::NONE;
                    }
                }
                if (!app.ready) {
                    GuiDisable();
                }
                if (GuiButton(Rectangle(active_dropdown_x, 32.0f, TOP_DROPDOWN_BTN_SIZE, UI_VERT_PADDING), "Close ROM")) {
                    app.ready = false;
                    app.rom_name = "None";
                    app.emu->init();
                    current_dropdown = Menu::NONE;
                }
                GuiEnable();
                break;
            }
            case Menu::EMULATION: {
                if (!app.ready) {
                    GuiDisable();
                }
                GuiToggle(Rectangle(active_dropdown_x, 16.0f, TOP_DROPDOWN_BTN_SIZE, UI_VERT_PADDING), "Pause", &app.paused);
                if (GuiButton(Rectangle(active_dropdown_x, 32.0f, TOP_DROPDOWN_BTN_SIZE, UI_VERT_PADDING), "Reset")) {
                    app.ready = false;
                    app.emu->init();
                    app.emu->load_rom(app.rom_path);
                    app.ready = true;
                    current_dropdown = Menu::NONE;
                }
                GuiEnable();
                GuiDisable();
                if (GuiButton(Rectangle(active_dropdown_x, 48.0f, TOP_DROPDOWN_BTN_SIZE, UI_VERT_PADDING), "Save State")) {

                }
                if (GuiButton(Rectangle(active_dropdown_x, 64.0f, TOP_DROPDOWN_BTN_SIZE, UI_VERT_PADDING), "Load State")) {

                }
                GuiEnable();
                break;
            }
            case Menu::SETTINGS: {
                if (current_popup == Menu::SETTINGS) {
                    current_popup = Menu::NONE;
                } else {
                    current_popup = Menu::SETTINGS;
                }
                current_dropdown = Menu::NONE;
                break;
            }
            case Menu::ABOUT: {
                if (current_popup == Menu::ABOUT) {
                    current_popup = Menu::NONE;
                } else {
                    current_popup = Menu::ABOUT;
                }
                current_dropdown = Menu::NONE;
                break;
            }
        }
        switch (current_popup) {
            case Menu::SETTINGS: {
                const Rectangle SettingsMenuRect = Rectangle(
                    static_cast<float>(window_width - SETTINGS_MENU_SIZE.x)/2.0f,
                    static_cast<float>(window_height - SETTINGS_MENU_SIZE.y)/2.0f,
                    SETTINGS_MENU_SIZE.x,
                    SETTINGS_MENU_SIZE.y
                );
                if ( GuiWindowBox(SettingsMenuRect, "#141#Settings") ) {
                    current_popup = Menu::NONE;
                }
                const float BUTTON_SIZE = ( SettingsMenuRect.width - ( UI_HORZ_PADDING * 3 ) ) / 2;
                auto cur = Vector2(SettingsMenuRect.x + UI_HORZ_PADDING, SettingsMenuRect.y + 32.0f);
                GuiLine(Rectangle(SettingsMenuRect.x, cur.y, SettingsMenuRect.width, 2.0f), "Emulation");
                cur.y += 12.0f;
                GuiDrawText("Hz:", Rectangle(cur.x, cur.y, BUTTON_SIZE, 8.0f), TEXT_ALIGN_CENTER, text_color);
                cur.y += 10.0f;
                if ( GuiValueBox(Rectangle(cur.x, cur.y, BUTTON_SIZE, 20.0f), "", &app.hz, 0, 1000000, hz_edit_mode) ) {
                    hz_edit_mode = !hz_edit_mode;
                }
                cur.x += BUTTON_SIZE + UI_HORZ_PADDING;
                cur.y -= 10.0f;
                GuiDrawText("Core:", Rectangle(cur.x, cur.y, BUTTON_SIZE, 8.0f), TEXT_ALIGN_CENTER, text_color);
                cur.y += 10.0f;
                const Rectangle core_dropdown_rect = {cur.x, cur.y, BUTTON_SIZE, 20.0f};
                cur.y += 28.0f;
                cur.x -= BUTTON_SIZE + UI_HORZ_PADDING;
                GuiCheckBox(Rectangle(cur.x, cur.y, 20.0f, 20.0f), "Unlocked Speed", &app.unlocked_speed);


                cur.y += 20.0f + (UI_VERT_PADDING / 2.0f);
                GuiLine(Rectangle(SettingsMenuRect.x, cur.y, SettingsMenuRect.width, 2.0f), "Video");
                cur.y += 12.0f;
                GuiDrawText("Max FPS:", Rectangle(cur.x, cur.y, BUTTON_SIZE, 8.0f), TEXT_ALIGN_CENTER, text_color);
                cur.y += 10.0f;
                if (vsync) {
                    GuiDisable();
                    temp_max_fps = max_fps;
                }
                if ( GuiValueBox(Rectangle(cur.x, cur.y, BUTTON_SIZE, 20.0f), "", &temp_max_fps, 10, 2000, fps_edit_mode) ) {
                    fps_edit_mode = !fps_edit_mode;
                }
                if (vsync) {
                    GuiEnable();
                }
                cur.x += BUTTON_SIZE + UI_HORZ_PADDING;
                GuiCheckBox(Rectangle(cur.x, cur.y, 20.0f, 20.0f), "V-Sync", &vsync);

                cur.x -= BUTTON_SIZE + UI_HORZ_PADDING;
                cur.y += 20.0f + (UI_VERT_PADDING / 2.0f);
                GuiLine(Rectangle(SettingsMenuRect.x, cur.y, SettingsMenuRect.width, 2.0f), "Color Palette");
                cur.y += UI_VERT_PADDING / 2.0f;
                for (int i = 0; i < 4; i++) {
                    if ( GuiButton(Rectangle(cur.x, cur.y, 20.0f, 20.0f), "") ) {
                        selected_color_picker = i;
                    }
                    DrawRectangleRec(Rectangle(cur.x + 1.0f, cur.y + 1.0f, 18.0f, 18.0f), app.palette[i]);
                    if (selected_color_picker == i) {
                        DrawRectangleLinesEx(Rectangle(cur.x, cur.y, 20.0f, 20.0f), 1.0f, GetColor(GuiGetStyle( BUTTON, BORDER_COLOR_PRESSED )));
                    }
                    cur.x += 20.0f + (UI_HORZ_PADDING - 2); // ugly but required for it to be aligned.
                }

                cur.x = SettingsMenuRect.x + SettingsMenuRect.width - 84.0f - UI_HORZ_PADDING;
                GuiColorPicker(Rectangle(cur.x, cur.y, 60.0f, 60.0f), "Color", &app.palette[selected_color_picker]);

                if (GuiDropdownBox(core_dropdown_rect, "Chip8;SChip", &selected_core, core_dropdown_edit_mode)) {
                    core_dropdown_edit_mode = !core_dropdown_edit_mode;
                }
                break;
            }
            case Menu::ABOUT: {
                const Rectangle AboutMenuRect = Rectangle(
                    static_cast<float>(window_width - ABOUT_MENU_SIZE.x)/2.0f,
                    static_cast<float>(window_height - ABOUT_MENU_SIZE.y)/2.0f,
                    ABOUT_MENU_SIZE.x,
                    ABOUT_MENU_SIZE.y
                );
                if ( GuiWindowBox(AboutMenuRect, "#193#About") ) {
                    current_popup = Menu::NONE;
                }
                auto cur = Vector2(AboutMenuRect.x + UI_HORZ_PADDING, AboutMenuRect.y + 32.0f);

                DrawTexturePro(logo_tex, Rectangle(0, 0, 16, 16), Rectangle(cur.x, cur.y, 32, 32), Vector2(0, 0), 0, WHITE);
                cur.x += 32;
                cur.y += 2;
                GuiDrawText(
                    "Chip++",
                    Rectangle(cur.x, cur.y, AboutMenuRect.x + AboutMenuRect.width - cur.x, 8),
                    TEXT_ALIGN_CENTER,
                    text_color
                );
                cur.x += 4;
                cur.y += UI_VERT_PADDING * 0.6f;
                GuiDrawText(
                    "A Chip8[+] emulator.",
                    Rectangle(cur.x, cur.y, AboutMenuRect.x + AboutMenuRect.width - cur.x, 8),
                    TEXT_ALIGN_CENTER,
                    GetColor( GuiGetStyle( LABEL, TEXT_COLOR_DISABLED ) )
                );

                cur.y += UI_VERT_PADDING * 2;
                cur.x -= 2;

                GuiDrawText(
                    "Made possible by:",
                    Rectangle(cur.x, cur.y, AboutMenuRect.x + AboutMenuRect.width - cur.x, 8),
                    TEXT_ALIGN_CENTER,
                    text_color
                );
                cur.y += UI_VERT_PADDING * 2;
                GuiDrawText(
                    "- Raylib\n- Raygui\n- portable-file-dialogs",
                    Rectangle(cur.x, cur.y, AboutMenuRect.x + AboutMenuRect.width - cur.x, 8),
                    TEXT_ALIGN_LEFT,
                    text_color
                );

                cur.y = AboutMenuRect.y + AboutMenuRect.height - UI_VERT_PADDING;
                GuiDrawText(
                    "Thank you all!",
                    Rectangle(cur.x, cur.y, AboutMenuRect.x + AboutMenuRect.width - cur.x, 8),
                    TEXT_ALIGN_CENTER,
                    text_color
                );

                break;
            }
            default: {
                break;
            }
        }
        apply_core(app, selected_core);
        if (not fps_edit_mode) {
            max_fps = temp_max_fps;
        }
        ui_focus = hz_edit_mode || fps_edit_mode;
    }

    void apply_core(App& app, const int core) {
        if (core != current_core) {
            switch (core) {
                case 0: {
                    app.emu = std::make_unique<Chip8>();
                    break;
                };
                case 1: {
                    app.emu = std::make_unique<SChip>();
                    break;
                };
                default: {
                    break;
                }
            }
            apply_quirks(app, core);
            app.emu->init();
            if (app.ready && !app.rom_path.empty()) {
                app.emu->load_rom(app.rom_path);
            }
            current_core = core;
        }
    }
    void apply_quirks(const App& app, const int core) {
        switch (core) {
            case 0: {
                app.emu->quirks.emulate_old_shift = true;
                app.emu->quirks.emulate_buggy_jump_offset = false;
                app.emu->quirks.emulate_old_load_store = true;
                app.emu->quirks.emulate_vf_reset = true;
                app.emu->quirks.emulate_vblank_wait = true;
                break;
            };
            case 1: {
                app.emu->quirks.emulate_old_shift = false;
                app.emu->quirks.emulate_buggy_jump_offset = true;
                app.emu->quirks.emulate_old_load_store = false;
                app.emu->quirks.emulate_vf_reset = false;
                app.emu->quirks.emulate_vblank_wait = false;
                break;
            }
            default: {
                break;
            }
        }
    }
}