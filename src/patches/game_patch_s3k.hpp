#pragma once

#include "game_patch.hpp"
#include "../ui/engine.hpp"

class GamePatchS3K : public GamePatch {
private:
    static constexpr uint16_t CURRENT_ZONE_AND_ACT = 0xFE10;
    static constexpr uint16_t APPARENT_ZONE_AND_ACT = 0xEE4E;
    static constexpr uint16_t SAVED_ZONE_AND_ACT = 0xFE2C;
    static constexpr uint16_t SAVED_APPARENT_ZONE_AND_ACT = 0xFF9A;
    static constexpr uint16_t SAVED_CAMERA_MAX_Y_POS = 0xFE44;
    static constexpr uint16_t SAVED2_CAMERA_MAX_Y_POS = 0xFE62;
    static constexpr uint16_t RING_COUNT = 0xFE20;
    static constexpr uint16_t TIMER = 0xFE22;
    static constexpr uint16_t SCORE = 0xFE26;
    static constexpr uint16_t CONTINUE_COUNT = 0xFE18;
    static constexpr uint16_t DEMO_MODE_FLAG = 0xFFD0;
    static constexpr uint16_t COMPETITION_SETTINGS = 0xFF8A;
    static constexpr uint16_t COMPETITION_MODE = 0xFFE8;
    static constexpr uint16_t GAME_MODE = 0xF600;
    static constexpr uint16_t LIFE_COUNT = 0xFE12;
    static constexpr uint16_t NEXT_EXTRA_LIFE_SCORE = 0xFFC0;
    static constexpr uint16_t CHARACTER_PLAYED = 0xFF0A; ///< 0000 = Sonic & Tails, 0001 = Sonic, 0002 = Tails, 0003 = Knuckles
    static constexpr uint16_t EMERALDS_COUNT = 0xFFB0;
    static constexpr uint16_t SUPER_EMERALDS_COUNT = 0xFFB1;
    static constexpr uint16_t LAST_STARPOLE_HIT = 0xFE2A;
    static constexpr uint16_t LEVEL_STARTED_FLAG = 0xF711;
    static constexpr uint16_t SAVE_POINTER = 0xE660;
    static constexpr uint16_t CAMERA_X_POS = 0xEE78;
    static constexpr uint16_t CAMERA_Y_POS = 0xEE7C;
    static constexpr uint16_t COLLECTED_BIG_RINGS_ARRAY = 0xFF92;
    static constexpr uint16_t RING_COUNT_P2 = 0xFED0;
    static constexpr uint16_t TIMER_P2 = 0xFED2;
    static constexpr uint16_t SCORE_P2 = 0xFED6;
    static constexpr uint16_t LIFE_COUNT_P2 = 0xFEC6;
    static constexpr uint16_t NEXT_EXTRA_LIFE_SCORE_P2 = 0xFFC4;

    mdui::Engine* _engine = nullptr;

public:
    GamePatchS3K() = default;
    virtual ~GamePatchS3K() { delete _engine; }

    std::string game_code() override { return "s3k"; }
    std::string game_name() override { return "Sonic 3 & Knuckles"; }
    std::string rom_default_filename() override { return "Sonic & Knuckles + Sonic The Hedgehog 3 (World).md"; }

    void mark_empty_chunks(md::ROM& rom) override;
    void skip_title_screen_to_level_select(md::ROM& rom) override;
    void give_infinite_lives(md::ROM& rom) override;
    void add_savestate_handling(md::ROM& rom) override;
    void display_additionnal_hud_on_pause(md::ROM& rom) override;

    void game_specific_initialization(md::ROM& rom) override
    {
        uint32_t func_s3k_preinit = inject_func_preinit_s3k(rom);
        _engine = new mdui::Engine(rom, func_s3k_preinit);
    }

    void game_specific_patches(md::ROM& rom) override {}

private:
    uint32_t inject_func_preinit_s3k(md::ROM& rom);

    uint32_t inject_main_menu_press_a_handler(md::ROM& rom);
    uint32_t inject_main_menu(md::ROM& rom);

    uint32_t inject_play_menu_press_b_handler(md::ROM& rom);
    uint32_t inject_play_menu_press_a_handler(md::ROM& rom);
    uint32_t inject_play_menu(md::ROM& rom);

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
