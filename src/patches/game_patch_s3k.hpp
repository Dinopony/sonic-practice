#pragma once

#include "game_patch.hpp"

class GamePatchS3K : public GamePatch {
public:
    GamePatchS3K() = default;

    std::string game_code() override { return "s3k"; }
    std::string game_name() override { return "Sonic 3 & Knuckles"; }
    std::string rom_default_filename() override { return "Sonic & Knuckles + Sonic The Hedgehog 3 (World).md"; }

    void mark_empty_chunks(md::ROM& rom) override;

    void skip_title_screen_to_level_select(md::ROM& rom) override;
    void give_infinite_lives(md::ROM& rom) override;
    void add_savestate_handling(md::ROM& rom) override;
    void display_additionnal_hud_on_pause(md::ROM& rom) override;

    void add_settings_menu(md::ROM& rom);
    void game_specific_patches(md::ROM& rom) override
    {
        add_settings_menu(rom);
    }


    // TODO: Add a way to give Wheel Glitch
    // TODO: Add a way to give Slope Glitch
    // TODO: Add a way to have a timer precise to the tenth of seconds
    // TODO: Add a way to instantly count time bonus
    // TODO: Add a way to disable music
    // TODO: Add a way to quickly restart the level
    // TODO: Add a way to easily enable slowdown mechanics
    // TODO: Add a way to easily enable debug mode

    // TODO: Add a configurable menu to setup settings
};
